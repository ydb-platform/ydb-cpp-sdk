<?php
$connection = odbc_connect($argv[1] ?? getenv('ODBC_CONNECTION_STRING'), '', '');
if (!$connection) throw new RuntimeException(odbc_errormsg());
try {
    $statement = odbc_exec($connection, "SELECT 42 AS answer, 'Hello YDB' AS message");
    if (!$statement) throw new RuntimeException(odbc_errormsg($connection));
    while ($row = odbc_fetch_array($statement)) {
        echo implode("\t", $row), "\n";
    }
    odbc_free_result($statement);
} finally {
    odbc_close($connection);
}
