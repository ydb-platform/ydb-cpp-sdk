library(DBI)

main <- function() {
  args <- commandArgs(TRUE)
  connection_string <- if (length(args)) args[[1]] else Sys.getenv("ODBC_CONNECTION_STRING")
  stopifnot(nzchar(connection_string))
  connection <- dbConnect(odbc::odbc(), .connection_string = connection_string)
  on.exit(dbDisconnect(connection))
  result <- dbSendQuery(connection, "SELECT CAST(? AS Int32) AS answer, CAST(? AS Utf8) AS message")
  on.exit(dbClearResult(result), add = TRUE, after = FALSE)
  dbBind(result, list(42L, "Hello YDB"))
  row <- dbFetch(result)
  cat(sprintf("%s\t%s\n", row$answer, row$message), sep = "")
}

main()
