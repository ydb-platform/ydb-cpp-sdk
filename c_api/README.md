Синхронный API	Асинхронный API

libpq: 
```c
PGconn *conn = PQconnectdb("...");
// Блокирует выполнение до завершения
```

libpq: 
```c
PGconn *conn = PQconnectStart("...");
do {
    pollstatus = PQconnectPoll(conn);
    // Ожидание событий
} while (pollstatus != PGRES_POLLING_OK);
```

MySQL: 
<br>MYSQL *conn = mysql_init(NULL);
<br>mysql_real_connect(conn, ...);	

MySQL: 
<br>status = mysql_real_connect_nonblocking(mysql, ...);
<br>while (status == NET_ASYNC_NOT_READY) {
    <br>  // Обработка других задач<br>  
    status = mysql_real_connect_nonblocking(...);<br>
}

Выполнение запросов

Синхронный API	Асинхронный API

libpq: <br>PGresult *res = PQexec(conn, "SELECT ...");<br>Ожидает завершения выполнения	

libpq: <br>PQsendQuery(conn, "SELECT ...");<br>// Можно выполнять другую работу<br>while ((res = PQgetResult(conn)) != NULL) {<br>  // Обработка результатов<br>}

MySQL: <br>mysql_query(conn, "SELECT ...");<br>result = mysql_store_result(conn);	

MySQL: <br>status = mysql_real_query_nonblocking(mysql, "...");<br>// Проверка status и ожидание<br>status = mysql_store_result_nonblocking(mysql, &result);

Обработка ошибок

Синхронный API	Асинхронный API

libpq: <br>if (PQstatus(conn) != CONNECTION_OK) {<br>  fprintf(stderr, "%s", PQerrorMessage(conn));<br>}	

libpq: <br>Такая же проверка, но в каждом шаге асинхронного процесса:<br>if (pollstatus == PGRES_POLLING_FAILED) {<br>  fprintf(stderr, "%s", PQerrorMessage(conn));<br>}

MySQL: <br>if (mysql_query(conn, query)) {<br>  fprintf(stderr, "%s", mysql_error(conn));<br>}	

MySQL: <br>if (status == NET_ASYNC_ERROR) {<br>  fprintf(stderr, "%s", mysql_error(mysql));<br>}