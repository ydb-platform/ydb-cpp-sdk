library(DBI)

main <- function() {
  args <- commandArgs(TRUE)
  connection_string <- if (length(args)) args[[1]] else Sys.getenv("ODBC_CONNECTION_STRING")
  stopifnot(nzchar(connection_string))
  emit <- function(...) cat(paste(..., sep = "\t"), "\n", sep = "")
  connection <- dbConnect(odbc::odbc(), .connection_string = connection_string)
  table <- paste0("r_odbc_example_", Sys.getpid())
  created <- transaction <- FALSE
  result <- insert <- reader <- NULL
  on.exit({
    if (!is.null(result) && dbIsValid(result)) dbClearResult(result)
    if (!is.null(insert) && dbIsValid(insert)) dbClearResult(insert)
    if (!is.null(reader)) dbDisconnect(reader)
    if (transaction) dbRollback(connection)
    if (created) dbExecute(connection, paste("DROP TABLE", table))
    dbDisconnect(connection)
  })
  dbExecute(connection, paste("CREATE TABLE", table,
    "(id Int32 NOT NULL, name Utf8, note Utf8, quantity Int32, PRIMARY KEY (id))"))
  created <- TRUE
  insert <- dbSendStatement(connection, paste("INSERT INTO", table,
    "(id, name, note, quantity) VALUES (Unwrap(CAST(? AS Int32)), CAST(? AS Utf8), CAST(? AS Utf8), CAST(? AS Int32))"))
  dbBind(insert, list(1:3, c("Чай", "Café", "Water"),
                      c(NA_character_, "fresh", "still"), c(10L, 20L, 30L)))
  emit("inserted", dbGetRowsAffected(insert))
  dbClearResult(insert)
  result <- dbSendQuery(connection, paste("SELECT id, name, note, quantity FROM", table, "ORDER BY id"))
  emit(paste(c("columns", dbColumnInfo(result)$name), collapse = "\t"))
  rows <- dbFetch(result)
  for (i in seq_len(nrow(rows))) {
    values <- vapply(rows[i, ], function(value) if (is.na(value)) "NULL" else as.character(value), character(1))
    emit(paste(values, collapse = "\t"))
  }
  dbClearResult(result)
  emit("matching tables", sum(dbListTables(connection) == table))
  emit(paste(c("schema", dbListFields(connection, table)), collapse = "\t"))
  emit("updated", dbExecute(connection, paste("UPDATE", table, "SET quantity = 21 WHERE id = 2")))
  emit("deleted", dbExecute(connection, paste("DELETE FROM", table, "WHERE id = 999")))

  reader <- dbConnect(odbc::odbc(), .connection_string = connection_string)
  quantity_sql <- paste("SELECT quantity FROM", table, "WHERE id = 1")
  dbBegin(connection); transaction <- TRUE
  dbExecute(connection, paste("UPDATE", table, "SET quantity = 99 WHERE id = 1"))
  dbRollback(connection); transaction <- FALSE
  emit("after rollback", dbGetQuery(reader, quantity_sql)$quantity)
  dbBegin(connection); transaction <- TRUE
  dbExecute(connection, paste("UPDATE", table, "SET quantity = 11 WHERE id = 1"))
  dbCommit(connection); transaction <- FALSE
  emit("after commit", dbGetQuery(reader, quantity_sql)$quantity)

  # Stock nanodbc loses SQLSTATE; this exact YDB diagnostic means 40001.
  # Retry the complete transaction, including commit, and rollback before retrying.
  for (attempt in seq_len(3L)) {
    dbBegin(connection); transaction <- TRUE
    failure <- tryCatch({
      quantity <- dbGetQuery(connection, quantity_sql)$quantity
      # A competing connection creates one real serialization conflict.
      if (attempt == 1L) dbExecute(reader, paste("UPDATE", table, "SET quantity = quantity + 1 WHERE id = 1"))
      dbExecute(connection, paste("UPDATE", table, "SET quantity =", quantity + 5L, "WHERE id = 1"))
      dbCommit(connection); transaction <- FALSE
      NULL
    }, error = identity)
    if (is.null(failure)) break
    dbRollback(connection); transaction <- FALSE
    if (attempt == 3L || !grepl("^[^\n]*\nSerialization failure:", conditionMessage(failure))) stop(failure)
  }
  emit("transaction attempts", attempt)
  emit("after retry", dbGetQuery(reader, quantity_sql)$quantity)

  # Rebinding abandons unread rows and reuses the prepared query.
  result <- dbSendQuery(connection, paste("SELECT id, name FROM", table,
    "WHERE id >= CAST(? AS Int32) ORDER BY id"))
  dbBind(result, list(1L))
  first <- dbFetch(result, n = 1)
  emit("first", first$id, first$name)
  emit("independent reader", dbGetQuery(reader, paste("SELECT COUNT(*) AS total FROM", table))$total)
  dbBind(result, list(2L))
  rows <- dbFetch(result)
  for (i in seq_len(nrow(rows))) emit("rebound", rows$id[i], rows$name[i])
  dbClearResult(result)
  emit("deleted existing", dbExecute(connection, paste("DELETE FROM", table, "WHERE id = 3")))
  emit("remaining", dbGetQuery(connection, paste("SELECT COUNT(*) AS total FROM", table))$total)
}

main()
