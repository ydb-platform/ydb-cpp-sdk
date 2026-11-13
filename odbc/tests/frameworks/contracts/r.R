library(DBI)

args <- commandArgs(TRUE)
mode <- args[[1]]
connection <- args[[2]]
stopifnot(mode %in% c("dsn", "connection_string"))

connect <- function() {
  if (mode == "dsn") dbConnect(odbc::odbc(), dsn = connection)
  else dbConnect(odbc::odbc(), .connection_string = connection)
}
check <- function(ok, message) if (!isTRUE(ok)) stop(message, call. = FALSE)
with_connection <- function(f) {
  con <- connect()
  on.exit(if (dbIsValid(con)) dbDisconnect(con))
  f(con)
}
with_table <- function(con, case, f) {
  table <- paste0("contract_r_", case)
  dbExecute(con, paste("DROP TABLE IF EXISTS", table))
  dbExecute(con, paste("CREATE TABLE", table,
                       "(id Int32, value Utf8, PRIMARY KEY (id))"))
  on.exit(dbExecute(con, paste("DROP TABLE IF EXISTS", table)))
  f(table)
}
with_result <- function(res, f) {
  on.exit(if (dbIsValid(res)) dbClearResult(res))
  f(res)
}
scalar <- function(con, sql) dbGetQuery(con, sql)[[1]][[1]]

cases <- list(
  connection = function() with_connection(function(con) {
    check(dbIsValid(con), "connection is invalid")
    check(identical(scalar(con, "SELECT 'connected'u AS value"), "connected"),
          "connection query differs")
  }),
  binding = function() with_connection(function(con) {
    unicode <- "\u041f\u0440\u0438\u0432\u0435\u0442 \U0001f30d"
    with_result(dbSendQuery(con,
      paste("SELECT CAST(? AS Int32) AS id, CAST(? AS Utf8) AS value,",
            "CAST(? AS Utf8) AS missing")), function(res) {
      dbBind(res, list(7L, unicode, NA_character_))
      row <- dbFetch(res)
      check(nrow(row) == 1L && row[[1]][[1]] == 7L && row[[2]][[1]] == unicode &&
              is.na(row[[3]][[1]]),
            paste("bound values differ:", paste(capture.output(dput(row)), collapse = " ")))
    })
  }),
  metadata = function() with_connection(function(con) {
    with_table(con, "metadata", function(table) {
      check(table %in% dbListTables(con), "table missing from catalog")
      check(identical(tolower(dbListFields(con, table)), c("id", "value")),
            "catalog columns differ")
      with_result(dbSendQuery(con, paste("SELECT id, value FROM", table)), function(res) {
        check(identical(tolower(dbColumnInfo(res)$name), c("id", "value")),
              "result columns differ")
      })
    })
  }),
  row_count = function() with_connection(function(con) {
    with_table(con, "row_count", function(table) {
      check(dbExecute(con, paste("UPSERT INTO", table,
        "(id, value) VALUES (1, 'one'u), (2, 'two'u), (3, 'three'u)")) == 3,
        "insert affected-row count differs")
      check(dbExecute(con, paste("UPDATE", table,
        "SET value = 'updated'u WHERE id <= 2")) == 2,
        "update affected-row count differs")
      check(dbExecute(con, paste("DELETE FROM", table, "WHERE id = 3")) == 1,
            "delete affected-row count differs")
      check(dbExecute(con, paste("DELETE FROM", table, "WHERE id = 99")) == 0,
            "no-op affected-row count differs")
    })
  }),
  transactions = function() with_connection(function(con) {
    with_table(con, "transactions", function(table) {
      dbExecute(con, paste("UPSERT INTO", table, "(id, value) VALUES (1, 'seed'u)"))
      active <- FALSE
      on.exit(if (active) dbRollback(con))
      dbBegin(con)
      active <- TRUE
      dbExecute(con, paste("UPDATE", table, "SET value = 'discarded'u WHERE id = 1"))
      dbRollback(con)
      active <- FALSE
      check(scalar(con, paste("SELECT value FROM", table, "WHERE id = 1")) == "seed",
            "rollback changed stored value")
      dbBegin(con)
      active <- TRUE
      dbExecute(con, paste("UPDATE", table, "SET value = 'committed'u WHERE id = 1"))
      dbCommit(con)
      active <- FALSE
      check(scalar(con, paste("SELECT value FROM", table, "WHERE id = 1")) == "committed",
            "commit did not persist value")
    })
  }),
  independent_connections = function() with_connection(function(first) {
    with_table(first, "independent_connections", function(table) {
      with_connection(function(second) {
        first_active <- second_active <- FALSE
        on.exit({
          if (second_active) dbRollback(second)
          if (first_active) dbRollback(first)
        })
        dbBegin(first)
        first_active <- TRUE
        dbExecute(first, paste("UPSERT INTO", table, "(id, value) VALUES (1, 'pending'u)"))
        check(nrow(dbGetQuery(second, paste("SELECT value FROM", table))) == 0L,
              "second connection saw an uncommitted write")
        dbBegin(second)
        second_active <- TRUE
        dbExecute(second, paste("UPSERT INTO", table, "(id, value) VALUES (2, 'committed'u)"))
        dbCommit(second)
        second_active <- FALSE
        dbRollback(first)
        first_active <- FALSE
        rows <- dbGetQuery(first, paste("SELECT value FROM", table))
        check(nrow(rows) == 1L && rows[[1]][[1]] == "committed",
              "rollback on first connection changed second connection's commit")
      })
    })
  }),
  cleanup = function() with_connection(function(con) {
    res <- dbSendQuery(con, "SELECT 1 AS value UNION ALL SELECT 2 AS value")
    with_result(res, function(result) {
      check(nrow(dbFetch(result, n = 1)) == 1L, "initial fetch failed")
    })
    check(!dbIsValid(res), "cleared result is still valid")
    check(scalar(con, "SELECT 'reused'u AS value") == "reused",
          "connection reuse after result cleanup failed")
    dbDisconnect(con)
    check(!dbIsValid(con), "disconnected connection is still valid")
  })
)

failed <- FALSE
for (name in names(cases)) {
  message <- tryCatch({ cases[[name]](); "" }, error = function(error) conditionMessage(error))
  status <- if (nzchar(message)) "failed" else "passed"
  failed <- failed || status == "failed"
  hex <- paste(sprintf("%02x", as.integer(charToRaw(enc2utf8(message)))), collapse = "")
  cat("@YDB_CONTRACT", name, status, hex, sep = "\t")
  cat("\n")
}
quit(status = as.integer(failed))
