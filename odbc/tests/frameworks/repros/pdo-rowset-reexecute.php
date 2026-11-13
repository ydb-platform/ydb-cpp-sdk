<?php
$pdo = new PDO('odbc:' . $argv[1], '', '', [PDO::ATTR_ERRMODE => PDO::ERRMODE_EXCEPTION]);
$statement = $pdo->prepare('SELECT 1 AS value');
$statement->execute();
var_dump($statement->columnCount(), $statement->fetch(PDO::FETCH_ASSOC));
var_dump($statement->nextRowset());
$statement->execute();
var_dump($statement->columnCount(), $statement->fetch(PDO::FETCH_ASSOC));
