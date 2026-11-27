library(DBI)
library(odbc)
con <- dbConnect(odbc(), .connection_string=commandArgs(TRUE)[[1]])
cases <- list(
    factor=list(factor("label"), "VARCHAR(20)", "label"),
    date_integer=list(structure(18618L, class="Date"), "DATE", "2020-12-22"),
    timestamp=list(as.POSIXct("2023-12-17 02:40:49", tz="UTC"), "TIMESTAMP", "2023-12-17 02:40:49"),
    time_hours=list(structure(1, class="difftime", units="hours"), "TIME", "01:00:00"),
    time_minutes=list(structure(1, class="difftime", units="mins"), "TIME", "00:01:00"))
for (name in names(cases)) {
    x <- cases[[name]]
    value <- tryCatch(dbGetQuery(con, paste0("SELECT CAST(? AS ",x[[2]],") AS value"), params=list(x[[1]])),
                      error=function(e) conditionMessage(e))
    literal <- dbQuoteLiteral(con, x[[1]])
    comparison <- tryCatch(dbGetQuery(con, paste0("SELECT CASE WHEN ? = ", literal,
                           " THEN 1.5 ELSE 2.5 END AS value"), params=list(x[[1]])),
                           error=function(e) conditionMessage(e))
    cat("comparison",name,"expected 1.5 returned",paste(capture.output(print(comparison)),collapse=" "),"\n")
    cat(name, "expected", x[[3]], "returned", paste(capture.output(print(value)),collapse=" "), "\n")
}
dbDisconnect(con)
