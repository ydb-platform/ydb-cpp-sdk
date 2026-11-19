<?php
$dsn = $argv[1] ?? getenv('ODBC_CONNECTION_STRING');
$connection = odbc_connect($dsn, '', '');
if (!$connection) throw new RuntimeException(odbc_errormsg());
$table = 'example_php_odbc_' . bin2hex(random_bytes(8));
$created = false;
$failure = null;
$exec = static function (string $sql) use ($connection) {
    $result = @odbc_exec($connection, $sql);
    if (!$result) throw new RuntimeException(odbc_error($connection) . ': ' . odbc_errormsg($connection));
    return $result;
};
try {
    $exec("CREATE TABLE $table (id Int32 NOT NULL, name Utf8, note Utf8, payload String, created Timestamp, amount Int32, PRIMARY KEY (id))");
    $created = true;
    $insert = odbc_prepare($connection, "INSERT INTO $table (id, name, note, payload, created, amount) VALUES (?, ?, ?, ?, ?, ?)");
    if (!$insert) throw new RuntimeException(odbc_errormsg($connection));
    foreach ([[1, 'Привет, 世界', null, "\0A\xff", '2024-06-15 14:30:20.123456', 10],
              [2, 'Second', str_repeat('abcdef', 1000), 'plain', '2024-06-16 01:02:03.000000', 20]] as $values) {
        if (!odbc_execute($insert, $values)) throw new RuntimeException(odbc_errormsg($connection));
        echo 'inserted: ', odbc_num_rows($insert), "\n";
    }
    $rows = $exec("SELECT id, name, note, payload, created, amount FROM $table ORDER BY id");
    odbc_longreadlen($rows, 16384);
    odbc_binmode($rows, ODBC_BINMODE_RETURN);
    $names = [];
    for ($i = 1; $i <= odbc_num_fields($rows); ++$i) $names[] = odbc_field_name($rows, $i);
    echo 'columns: ', implode(', ', $names), "\n";
    while ($row = odbc_fetch_array($rows)) {
        echo $row['id'], ': ', $row['name'], ', note=', $row['note'] === null ? 'NULL' : strlen($row['note']) . ':' . hash('sha256', $row['note']),
             ', payload=', bin2hex($row['payload']), ', created=', $row['created'], ', amount=', $row['amount'], "\n";
    }
    odbc_free_result($rows);
    echo 'updated: ', odbc_num_rows($exec("UPDATE $table SET name = 'Updated' WHERE id = 2")), "\n";
    echo 'updated missing: ', odbc_num_rows($exec("UPDATE $table SET name = 'Missing' WHERE id = 99")), "\n";
    if (!odbc_autocommit($connection, false)) throw new RuntimeException(odbc_errormsg($connection));
    $exec("UPDATE $table SET name = 'Rolled back' WHERE id = 1");
    if (!odbc_rollback($connection)) throw new RuntimeException(odbc_errormsg($connection));
    if (!odbc_autocommit($connection, true)) throw new RuntimeException(odbc_errormsg($connection));
    echo 'after rollback: ', odbc_fetch_array($exec("SELECT name FROM $table WHERE id = 1"))['name'], "\n";
    $other = odbc_connect($dsn, '', '');
    if (!$other) throw new RuntimeException(odbc_errormsg());
    try {
        for ($attempt = 1; $attempt <= 3; ++$attempt) {
            try {
                if (!odbc_autocommit($connection, false)) throw new RuntimeException(odbc_errormsg($connection));
                $amount = (int)odbc_fetch_array($exec("SELECT amount FROM $table WHERE id = 1"))['amount'];
                // The independent writer makes the first read/modify/write transaction conflict.
                if ($attempt === 1 && !odbc_exec($other, "UPDATE $table SET amount = amount + 1 WHERE id = 1"))
                    throw new RuntimeException(odbc_errormsg($other));
                $exec("UPDATE $table SET amount = " . ($amount + 1) . ' WHERE id = 1');
                if (!@odbc_commit($connection)) throw new RuntimeException(odbc_errormsg($connection));
                break;
            } catch (RuntimeException $error) {
                $state = odbc_error($connection);
                $rolledBack = odbc_rollback($connection);
                if (!$rolledBack || $state !== '40001' || $attempt === 3) throw $error;
                if (!odbc_autocommit($connection, true)) throw new RuntimeException(odbc_errormsg($connection));
                usleep(10000 * $attempt);
            }
        }
        if (!odbc_autocommit($connection, true)) throw new RuntimeException(odbc_errormsg($connection));
    } finally {
        odbc_close($other);
    }
    echo 'transaction attempts: ', $attempt, "\n";
    echo 'committed amount: ', odbc_fetch_array($exec("SELECT amount FROM $table WHERE id = 1"))['amount'], "\n";
    $select = odbc_prepare($connection, "SELECT id, name FROM $table ORDER BY id");
    if (!$select || !odbc_execute($select)) throw new RuntimeException(odbc_errormsg($connection));
    echo 'first row: ', odbc_fetch_array($select)['id'], "\n";
    if (!odbc_execute($select)) throw new RuntimeException(odbc_errormsg($connection));
    echo 'reused cursor: ', odbc_fetch_array($select)['id'], "\n";
    odbc_free_result($select);
    $tables = odbc_tables($connection, null, null, $table, 'TABLE');
    if (!$tables) throw new RuntimeException(odbc_errormsg($connection));
    echo 'table type: ', odbc_fetch_array($tables)['TABLE_TYPE'], "\n";
    odbc_free_result($tables);
    $columns = odbc_columns($connection, null, null, $table, '%');
    if (!$columns) throw new RuntimeException(odbc_errormsg($connection));
    while ($column = odbc_fetch_array($columns)) echo 'schema: ', $column['COLUMN_NAME'], ' ', $column['TYPE_NAME'], "\n";
    odbc_free_result($columns);
    $keys = odbc_primarykeys($connection, null, null, $table);
    if (!$keys) throw new RuntimeException(odbc_errormsg($connection));
    echo 'primary key: ', odbc_fetch_array($keys)['COLUMN_NAME'], "\n";
    odbc_free_result($keys);
    $batch = $exec('SELECT 10 AS value; SELECT 20 AS value');
    do {
        echo 'result: ', odbc_fetch_array($batch)['value'], "\n";
    } while (odbc_next_result($batch));
    odbc_free_result($batch);
    if (@odbc_exec($connection, 'SELECT FROM') !== false) throw new RuntimeException('Invalid SQL unexpectedly succeeded');
    echo 'invalid SQL: ', odbc_error($connection), "\n";
    echo 'connection reused: ', odbc_fetch_array($exec('SELECT 42 AS answer'))['answer'], "\n";
    echo 'deleted: ', odbc_num_rows($exec("DELETE FROM $table WHERE id = 2")), "\n";
    echo 'deleted missing: ', odbc_num_rows($exec("DELETE FROM $table WHERE id = 99")), "\n";
    echo 'remaining: ', odbc_fetch_array($exec("SELECT COUNT(*) AS total FROM $table"))['total'], "\n";
} catch (Throwable $error) {
    $failure = $error;
    throw $error;
} finally {
    try {
        if (!@odbc_rollback($connection) || !@odbc_autocommit($connection, true)
            || ($created && !@odbc_exec($connection, "DROP TABLE $table"))) {
            $error = new RuntimeException('Cleanup: ' . odbc_errormsg($connection));
            if ($failure === null) throw $error;
            fwrite(STDERR, $error->getMessage() . "\n");
        }
    } finally {
        odbc_close($connection);
    }
}
