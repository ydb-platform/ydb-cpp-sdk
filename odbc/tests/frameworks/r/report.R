library(DBI)
library(odbc)
library(DBItest)
library(testthat)
native <- Sys.getenv("ODBC_NATIVE_RESULTS_DIR")
Sys.setenv(ODBC_CS_YDB=Sys.getenv("YDB_R_CONNECTION"))
# DBItest's standard ODBC contract. Arrow and stress are separate API suites.
categories <- c("getting_started", "driver", "connection", "result", "sql", "meta", "transaction", "compliance")
specs <- unlist(lapply(categories, function(name) get(paste0("spec_", name), asNamespace("DBItest"))), recursive=FALSE)
specs <- specs[!vapply(specs,is.null,logical(1))]
names(specs) <- vctrs::vec_names2(specs, repair="unique", quiet=TRUE)
specs <- specs[!grepl("arrow|stream_bind",names(specs))]
source_dir <- "/work/source/tests/testthat"
files <- list.files(source_dir, pattern="^test-(odbc-|utils|dbi-connection).*\\.R$", full.names=TRUE)
# These regression cases exercise the common ODBC path despite living beside
# backend-specific tests. Keep the backend DBItest wrappers out of this run.
backend_cases <- list(
    "test-driver-mysql.R" = c("can roundtrip columns", "odbcPreviewObject"),
    "test-driver-oracle.R" = c("can round columns", "can detect existence of table"),
    "test-driver-postgres.R" = c("can roundtrip columns", "64 bit integers work with alternate mappings",
        "Writing data.frame with column ordering different than target table", "odbcPreviewObject"),
    "test-driver-sql-server.R" = c("works with dbAppendTable (#215)",
        "Subseconds are retained upon insertion (#208)", "blobs can be retrieved out of order",
        "can bind NA values", "odbcPreviewObject doesn't warn about pending rows",
        "dates should always be interpreted in the system time zone (#398)",
        "UTF in VARCHAR is not truncated", "Zero-row-fetch does not move cursor",
        "captures multiline errors message"),
    "test-driver-sqlite.R" = c("unsupported types gives informative error", "odbcPreviewObject works"))
backend_expressions <- lapply(names(backend_cases), function(file) {
    expressions <- parse(file.path(source_dir, file), keep.source=TRUE, encoding="UTF-8")
    selected <- Filter(function(expr) is.call(expr) && identical(expr[[1]], as.name("test_that")) &&
        is.character(expr[[2]]) && expr[[2]] %in% backend_cases[[file]], as.list(expressions))
    actual <- vapply(selected, function(expr) expr[[2]], "")
    stopifnot(identical(actual, backend_cases[[file]]))
    selected
})
names(backend_expressions) <- names(backend_cases)
collect_names <- function(expr) {
    if (!is.call(expr) && !is.expression(expr)) return(character())
    if (is.call(expr) && identical(expr[[1]],as.name("test_that"))) {
        if (!is.character(expr[[2]])) stop("Cannot discover dynamic upstream test name")
        return(expr[[2]])
    }
    unlist(lapply(as.list(expr), collect_names), use.names=FALSE)
}
unit_names <- unlist(lapply(files,function(file) paste0(basename(file), ":", collect_names(parse(file)))))
unit_names <- c(unit_names, unlist(lapply(names(backend_cases), function(file)
    paste0(file, ":", backend_cases[[file]])), use.names=FALSE))
jsonlite::write_json(c(names(specs),unit_names), file.path(native,"r-inventory.json"), auto_unbox=FALSE)
if ("--list" %in% commandArgs(TRUE)) quit(status=0)
context <- DBItest::make_context(odbc::odbc(),
    list(.connection_string=Sys.getenv("YDB_R_CONNECTION"), encoding="UTF-8"), name="YDB", default_skip=NULL,
    tweaks=DBItest::tweaks(dbitest_version="1.8.0", union=function(x) paste(x,collapse=" UNION ALL "),
        # DBItest needs a valid DDL statement here; these cases do not inspect table data.
        create_table_as=function(table_name, query)
            paste0("CREATE TABLE ",table_name," (a Int32, PRIMARY KEY (a))"),
        placeholder_pattern="?", date_cast=function(x) paste0("Date32('",x,"'s)"),
        time_cast=function(x) paste0("Utf8('",x,"'s)"),
        blob_cast=function(x) {
            if (all(x == "NULL")) return(x)
            if (all(x == "?")) return(paste0("CAST(",x," AS String)"))
            if (all(grepl("^String::HexDecode",x))) return(x)
            hex <- sub("^(?:[xX]'([0-9A-Fa-f]*)'|0[xX]([0-9A-Fa-f]*))$", "\\1\\2",as.character(x),perl=TRUE)
            stopifnot(all(grepl("^[0-9A-Fa-f]*$",hex)))
            paste0("String::HexDecode('",hex,"'s)")
        },
        timestamp_cast=function(x) paste0("Timestamp64('",sub(" ","T",x,fixed=TRUE),"Z's)")))
reporter <- testthat::ListReporter$new()
results <- list()
for (name in names(specs)) {
    collector <- testthat::ListReporter$new()
    testthat::with_reporter(collector, {
        DBItest:::run_tests(context, specs[name], skip=NULL, run_only=NULL, test_suite="YDB")
    })
    cases <- collector$get_results()
    problems <- unlist(lapply(cases,function(case) lapply(Filter(function(result)
        !inherits(result,"expectation_success"), case$results),function(result)
            list(message=conditionMessage(result)))),recursive=FALSE)
    failed <- any(vapply(unlist(lapply(cases,`[[`,"results"),recursive=FALSE),function(result)
        inherits(result,c("expectation_failure","expectation_error","expectation_warning")),logical(1)))
    results[[length(results)+1L]] <- list(name=name,
        status=if(!length(cases)) "broken" else if(failed) "failed" else if(length(problems)) "skipped" else "passed",
        message=if(!length(cases)) "DBItest produced no result" else
            paste(vapply(problems,`[[`,"", "message"),collapse="\n"), problems=problems)
    jsonlite::write_json(results,file.path(native,"r-results.json"),auto_unbox=TRUE,pretty=TRUE)
}

# Run the binding's generic upstream tests as well as the DBI contract.
source(file.path(source_dir,"helper.R"),local=TRUE)
for (file in c(files, file.path(source_dir, names(backend_cases)))) {
    env <- new.env(parent=globalenv())
    for (symbol in ls(asNamespace("odbc"),all.names=TRUE)) assign(symbol,get(symbol,asNamespace("odbc")),env)
    if (basename(file) %in% names(backend_expressions)) {
        cases <- lapply(backend_expressions[[basename(file)]], function(expr) {
            collector <- testthat::ListReporter$new()
            testthat::with_reporter(collector, eval(expr, env))
            # test_roundtrip() has its own nested test_that(). Preserve every
            # expectation under the discovered outer regression case.
            list(test=expr[[2]], results=unlist(lapply(collector$get_results(), `[[`, "results"), recursive=FALSE))
        })
    } else {
        collector <- testthat::ListReporter$new()
        testthat::test_file(file, reporter=collector, env=env)
        cases <- collector$get_results()
    }
    for (case in cases) {
        problems <- lapply(Filter(function(result) !inherits(result,"expectation_success"),case$results),
                           function(result) list(message=conditionMessage(result)))
        failed <- any(vapply(case$results,function(result)
                         inherits(result,c("expectation_failure","expectation_error","expectation_warning")),logical(1)))
        results[[length(results)+1L]] <- list(name=paste0(basename(file),":",case$test),
            status=if(failed) "failed" else if(length(problems)) "skipped" else "passed",
            message=paste(vapply(problems,`[[`,"","message"),collapse="\n"),problems=problems)
    }
    jsonlite::write_json(results,file.path(native,"r-results.json"),auto_unbox=TRUE,pretty=TRUE)
}
