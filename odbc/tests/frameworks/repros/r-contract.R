library(DBI)
library(odbc)
library(DBItest)
library(testthat)
native <- commandArgs(TRUE)[[2]]
dir.create(native, recursive=TRUE, showWarnings=FALSE)
# Arrow requires a separate binding API; stress uses ordinary DBI connections.
categories <- c("getting_started", "driver", "connection", "result", "sql", "meta", "transaction", "compliance", "stress")
specs <- unlist(lapply(categories, function(name) get(paste0("spec_", name), asNamespace("DBItest"))), recursive=FALSE)
specs <- specs[!vapply(specs,is.null,logical(1))]
names(specs) <- vctrs::vec_names2(specs, repair="unique", quiet=TRUE)
specs <- specs[!grepl("arrow|stream_bind",names(specs))]
context <- DBItest::make_context(odbc::odbc(),
    list(.connection_string=commandArgs(TRUE)[[1]]), name="SQLite", default_skip=NULL,
    tweaks=DBItest::tweaks(dbitest_version="1.8.0", placeholder_pattern="?"))
selected <- commandArgs(TRUE)[-(1:2)]
if (length(selected)) specs <- specs[names(specs) %in% selected]
# Keep a real table visible: an empty catalog hides the identifier round-trip defect.
con <- dbConnect(odbc::odbc(), .connection_string=commandArgs(TRUE)[[1]])
dbExecute(con, "DROP TABLE IF EXISTS odbc_control_metadata")
dbExecute(con, "CREATE TABLE odbc_control_metadata (a INTEGER)")
dbDisconnect(con)
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

