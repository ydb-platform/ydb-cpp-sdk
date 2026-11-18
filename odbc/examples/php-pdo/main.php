<?php
$dsn = $argv[1] ?? getenv('ODBC_CONNECTION_STRING');
$options = [PDO::ATTR_ERRMODE => PDO::ERRMODE_EXCEPTION];
$connection = new PDO('odbc:' . $dsn, '', '', $options);
$table = 'example_php_pdo_' . bin2hex(random_bytes(8));
$created = false;
$failure = null;
try {
    $connection->exec("CREATE TABLE $table (id Int32 NOT NULL, name Utf8, note Utf8, payload String, created Timestamp, amount Int32, PRIMARY KEY (id))");
    $created = true;
    $insert = $connection->prepare("INSERT INTO $table (id, name, note, payload, created, amount) VALUES (?, ?, ?, ?, ?, ?)");
    foreach ([[1, 'Привет, 世界', null, "\0A\xff", '2024-06-15 14:30:20.123456', 10],
              [2, 'Second', str_repeat('abcdef', 1000), 'plain', '2024-06-16 01:02:03.000000', 20]] as $values) {
        $insert->bindValue(1, $values[0], PDO::PARAM_INT);
        $insert->bindValue(2, $values[1], PDO::PARAM_STR);
        $insert->bindValue(3, $values[2], $values[2] === null ? PDO::PARAM_NULL : PDO::PARAM_STR);
        $insert->bindValue(4, $values[3], PDO::PARAM_LOB);
        $insert->bindValue(5, $values[4], PDO::PARAM_STR);
        $insert->bindValue(6, $values[5], PDO::PARAM_INT);
        $insert->execute();
        echo 'inserted: ', $insert->rowCount(), "\n";
    }
    $select = $connection->prepare("SELECT id, name, note, payload, created, amount FROM $table ORDER BY id");
    $select->execute();
    $names = [];
    for ($i = 0; $i < $select->columnCount(); ++$i) $names[] = $select->getColumnMeta($i)['name'];
    echo 'columns: ', implode(', ', $names), "\n";
    while ($row = $select->fetch(PDO::FETCH_ASSOC)) {
        // PDO_ODBC fetches binary columns as hexadecimal text.
        echo $row['id'], ': ', $row['name'], ', note=', $row['note'] === null ? 'NULL' : strlen($row['note']) . ':' . hash('sha256', $row['note']),
             ', payload=', $row['payload'], ', created=', $row['created'], ', amount=', $row['amount'], "\n";
    }
    $select->closeCursor();
    echo 'updated: ', $connection->exec("UPDATE $table SET name = 'Updated' WHERE id = 2"), "\n";
    echo 'updated missing: ', $connection->exec("UPDATE $table SET name = 'Missing' WHERE id = 99"), "\n";
    $connection->beginTransaction();
    $connection->exec("UPDATE $table SET name = 'Rolled back' WHERE id = 1");
    $connection->rollBack();
    echo 'after rollback: ', $connection->query("SELECT name FROM $table WHERE id = 1")->fetchColumn(), "\n";
    $other = new PDO('odbc:' . $dsn, '', '', $options);
    for ($attempt = 1; $attempt <= 3; ++$attempt) {
        try {
            $connection->beginTransaction();
            $amount = (int)$connection->query("SELECT amount FROM $table WHERE id = 1")->fetchColumn();
            // A concurrent writer makes the first read/modify/write transaction conflict.
            if ($attempt === 1) $other->exec("UPDATE $table SET amount = amount + 1 WHERE id = 1");
            $connection->exec("UPDATE $table SET amount = " . ($amount + 1) . ' WHERE id = 1');
            $connection->commit();
            break;
        } catch (PDOException $error) {
            $state = $error->errorInfo[0];
            if ($connection->inTransaction()) $connection->rollBack();
            if ($state !== '40001' || $attempt === 3) throw $error;
            usleep(10000 * $attempt);
        }
    }
    echo 'transaction attempts: ', $attempt, "\n";
    echo 'committed amount: ', $connection->query("SELECT amount FROM $table WHERE id = 1")->fetchColumn(), "\n";
    $select->execute();
    echo 'first row: ', $select->fetchColumn(), "\n";
    $select->closeCursor();
    $select->execute();
    echo 'reused cursor: ', $select->fetchColumn(), "\n";
    $select->closeCursor();
    $batch = $connection->query('SELECT 10 AS value; SELECT 20 AS value');
    do {
        echo 'result: ', $batch->fetchColumn(), "\n";
    } while ($batch->nextRowset());
    $batch->closeCursor();
    try {
        $connection->query('SELECT FROM');
        throw new RuntimeException('Invalid SQL unexpectedly succeeded');
    } catch (PDOException $error) {
        echo 'invalid SQL: ', $error->errorInfo[0], "\n";
    }
    echo 'connection reused: ', $connection->query('SELECT 42 AS answer')->fetchColumn(), "\n";
    echo 'deleted: ', $connection->exec("DELETE FROM $table WHERE id = 2"), "\n";
    echo 'deleted missing: ', $connection->exec("DELETE FROM $table WHERE id = 99"), "\n";
    echo 'remaining: ', $connection->query("SELECT COUNT(*) FROM $table")->fetchColumn(), "\n";
} catch (Throwable $error) {
    $failure = $error;
    throw $error;
} finally {
    try {
        if ($connection->inTransaction()) $connection->rollBack();
        if ($created) $connection->exec("DROP TABLE $table");
    } catch (Throwable $error) {
        if ($failure === null) throw $error;
        fwrite(STDERR, 'Cleanup: ' . $error->getMessage() . "\n");
    }
}
