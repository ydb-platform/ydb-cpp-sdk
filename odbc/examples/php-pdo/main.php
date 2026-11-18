<?php
$connection = new PDO('odbc:' . ($argv[1] ?? getenv('ODBC_CONNECTION_STRING')), '', '', [
    PDO::ATTR_ERRMODE => PDO::ERRMODE_EXCEPTION,
]);
$statement = $connection->prepare('SELECT CAST(? AS Int32) AS answer, CAST(? AS Utf8) AS message');
$statement->bindValue(1, 42, PDO::PARAM_INT);
$statement->bindValue(2, 'Hello YDB', PDO::PARAM_STR);
$statement->execute();
while ($row = $statement->fetch(PDO::FETCH_NUM)) {
    echo implode("\t", $row), "\n";
}
$statement->closeCursor();
