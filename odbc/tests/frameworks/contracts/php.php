<?php
// Public PHP ODBC/PDO contracts; the installed bindings are unmodified.
set_error_handler(function ($severity, $message, $file, $line) {
    if (error_reporting() & $severity) throw new ErrorException($message, 0, $severity, $file, $line);
    return false;
});
function require_contract($condition, $message) {
    if (!$condition) throw new RuntimeException($message);
}
class ContractDb {
    public $connection, $pdo, $statements = [], $active = false;
    function __construct($consumer) {
        $this->pdo = $consumer === 'php-pdo';
        $dsn = getenv('ODBC_TEST_MODE') === 'dsn' ? getenv('YDB_ODBC_DSN') : getenv('YDB_ODBC_CONNECTION_STRING');
        require_contract((bool)$dsn, 'Missing connection configuration');
        $this->connection = $this->pdo
            ? new PDO('odbc:' . $dsn, '', '', [PDO::ATTR_ERRMODE => PDO::ERRMODE_EXCEPTION, PDO::ATTR_PERSISTENT => false])
            : odbc_connect($dsn, '', '');
        require_contract((bool)$this->connection, 'Connection failed');
    }
    function prepare($sql) {
        $s = $this->pdo ? $this->connection->prepare($sql) : odbc_prepare($this->connection, $sql);
        require_contract((bool)$s, 'Prepare failed: ' . $sql); $this->statements[] = $s; return $s;
    }
    function execute($s, $parameters = []) {
        if ($this->pdo) {
            foreach ($parameters as $i => $value)
                $s->bindValue($i + 1, $value, $value === null ? PDO::PARAM_NULL : (is_int($value) ? PDO::PARAM_INT : PDO::PARAM_STR));
            require_contract($s->execute(), 'Execute failed');
        } else require_contract(odbc_execute($s, $parameters), 'Execute failed');
    }
    function query($sql, $parameters = []) { $s = $this->prepare($sql); $this->execute($s, $parameters); return $s; }
    function fetch($s) { return $this->pdo ? $s->fetch(PDO::FETCH_ASSOC) : odbc_fetch_array($s); }
    function count($s) { return $this->pdo ? $s->rowCount() : odbc_num_rows($s); }
    function next($s) { return $this->pdo ? $s->nextRowset() : odbc_next_result($s); }
    function fields($s) {
        $count = $this->pdo ? $s->columnCount() : odbc_num_fields($s); $names = [];
        for ($i = 0; $i < $count; ++$i)
            $names[] = $this->pdo ? $s->getColumnMeta($i)['name'] : odbc_field_name($s, $i + 1);
        return $names;
    }
    function release(&$s) {
        if ($this->pdo) {
            if ($s->columnCount() > 0) require_contract($s->closeCursor(), 'Cursor close failed');
        } else odbc_free_result($s);
        foreach ($this->statements as $i => $value) if ($value === $s) unset($this->statements[$i]);
        $s = null;
    }
    function exec($sql) { $s = $this->query($sql); $this->release($s); }
    function scalar($sql) {
        $s = $this->query($sql); $row = $this->fetch($s);
        require_contract(is_array($row) && count($row) === 1, 'Expected one-column row');
        $value = array_values($row)[0]; $this->release($s); return $value;
    }
    function begin() {
        require_contract($this->pdo ? $this->connection->beginTransaction() : odbc_autocommit($this->connection, false), 'Begin failed');
        $this->active = true;
    }
    function end($commit) {
        $ok = $this->pdo ? ($commit ? $this->connection->commit() : $this->connection->rollBack())
                        : ($commit ? odbc_commit($this->connection) : odbc_rollback($this->connection));
        require_contract($ok, 'Transaction completion failed'); $this->active = false;
        if (!$this->pdo) require_contract(odbc_autocommit($this->connection, true), 'Autocommit reset failed');
    }
    function reset() {
        foreach ($this->statements as $s) $this->release($s);
        if ($this->active) $this->end(false);
    }
    function close() {
        try { $this->reset(); }
        finally { $this->statements = []; if (!$this->pdo) odbc_close($this->connection); $this->connection = null; }
    }
}
$consumer = $argv[1] ?? ''; $failed = false;
$cases = ['connection', 'binding', 'metadata', 'row_count', 'transactions', 'independent_connections', 'cleanup', 'multiple_results'];
foreach ($cases as $case) {
    $db = $other = null; $table = null; $diagnostic = '';
    try {
        require_contract(in_array($consumer, ['php-odbc', 'php-pdo'], true), 'Expected php-odbc or php-pdo');
        $db = new ContractDb($consumer);
        if (in_array($case, ['binding', 'row_count', 'transactions', 'independent_connections'], true)) {
            $table = 'contract_' . str_replace('-', '_', $consumer) . '_' . $case;
            $db->exec("DROP TABLE IF EXISTS $table");
            $db->exec("CREATE TABLE $table (id Int32, txt Utf8, missing Utf8, PRIMARY KEY (id))");
        }
        switch ($case) {
            case 'connection':
                require_contract((int)$db->scalar('SELECT 42 AS answer') === 42, 'Scalar query differs'); break;
            case 'binding':
                $s = $db->query("INSERT INTO $table (id, txt, missing) VALUES (?, ?, ?)", [7, 'Привет, 世界', null]);
                $db->release($s); $s = $db->query("SELECT id, txt, missing FROM $table"); $row = $db->fetch($s);
                require_contract(is_array($row) && (int)$row['id'] === 7 && $row['txt'] === 'Привет, 世界'
                    && $row['missing'] === null && $db->fetch($s) === false, 'Bound integer/UTF-8/NULL differs: ' . json_encode($row)); break;
            case 'metadata':
                $s = $db->query("SELECT 7 AS answer, 'Привет' AS message, NULL AS absent");
                require_contract($db->fields($s) === ['answer', 'message', 'absent'], 'Column names/count differ');
                $row = $db->fetch($s);
                require_contract(array_keys($row) === ['answer', 'message', 'absent'] && (int)$row['answer'] === 7
                    && $row['message'] === 'Привет' && $row['absent'] === null, 'Metadata row differs'); break;
            case 'row_count':
                foreach ([["INSERT INTO $table (id, txt) VALUES (1, 'one'), (2, 'two')", 2],
                          ["UPDATE $table SET txt = 'changed' WHERE id = 1", 1],
                          ["DELETE FROM $table WHERE id = 2", 1],
                          ["UPDATE $table SET txt = 'missing' WHERE id = 99", 0]] as [$sql, $expected]) {
                    $s = $db->query($sql); $actual = $db->count($s);
                    require_contract($actual === $expected, "Affected rows: expected $expected, got $actual"); $db->release($s);
                }
                require_contract($db->scalar("SELECT txt FROM $table") === 'changed', 'DML result differs'); break;
            case 'transactions':
                $db->begin(); $db->exec("INSERT INTO $table (id, txt) VALUES (1, 'rollback')"); $db->end(false);
                require_contract((int)$db->scalar("SELECT COUNT(*) FROM $table") === 0, 'Rollback retained the row');
                $db->begin(); $db->exec("INSERT INTO $table (id, txt) VALUES (2, 'commit')"); $db->end(true);
                require_contract($db->scalar("SELECT txt FROM $table WHERE id = 2") === 'commit', 'Commit lost the row'); break;
            case 'independent_connections':
                $other = new ContractDb($consumer);
                require_contract($other->connection !== $db->connection, 'Connections share a public handle');
                $db->begin(); $db->exec("INSERT INTO $table (id, txt) VALUES (1, 'uncommitted')");
                require_contract((int)$other->scalar("SELECT COUNT(*) FROM $table") === 0, 'Uncommitted row leaked');
                $other->exec("INSERT INTO $table (id, txt) VALUES (2, 'independent')"); $db->end(false);
                require_contract($db->scalar("SELECT txt FROM $table") === 'independent', 'Rollback changed the other connection');
                $other->close(); $other = null;
                require_contract((int)$db->scalar('SELECT 42 AS answer') === 42, 'Closing one connection broke the other'); break;
            case 'cleanup':
                $s = $db->prepare('SELECT 1 AS item UNION ALL SELECT 2 AS item ORDER BY item'); $db->execute($s);
                require_contract((int)$db->fetch($s)['item'] === 1, 'Initial row differs'); $db->execute($s);
                require_contract((int)$db->fetch($s)['item'] === 1, 'Unread reexecution differs'); $db->release($s);
                require_contract((int)$db->scalar('SELECT 42 AS answer') === 42, 'Unread cleanup prevented another query'); break;
            case 'multiple_results':
                $s = $db->query("SELECT 1 AS first_value UNION ALL SELECT 2 AS first_value ORDER BY first_value; SELECT 'second' AS label, 9 AS number; SELECT NULL AS missing");
                require_contract($db->fields($s) === ['first_value'] && (int)$db->fetch($s)['first_value'] === 1, 'First result differs');
                require_contract($db->next($s) && $db->fields($s) === ['label', 'number'], 'Unread transition metadata differs');
                $row = $db->fetch($s);
                require_contract($row['label'] === 'second' && (int)$row['number'] === 9, 'Second result differs');
                require_contract($db->next($s) && $db->fields($s) === ['missing'] && $db->fetch($s)['missing'] === null, 'Third result differs');
                require_contract(!$db->next($s) && !$db->next($s), 'Result end differs'); $db->execute($s);
                $fields = $db->fields($s); $row = $db->fetch($s);
                require_contract($fields === ['first_value'] && (int)$row['first_value'] === 1,
                    'Batch reexecution differs: ' . json_encode([$fields, $row])); break;
        }
    } catch (Throwable $error) { $diagnostic = get_class($error) . ': ' . $error->getMessage(); }
    finally {
        foreach ([$other, $db] as $connection) if ($connection) {
            try { $connection->reset(); } catch (Throwable $error) { $diagnostic .= ' Cleanup: ' . $error->getMessage(); }
        }
        try { if ($db && $table) $db->exec("DROP TABLE IF EXISTS $table"); }
        catch (Throwable $error) { $diagnostic .= ' Drop: ' . $error->getMessage(); }
        foreach ([$other, $db] as $connection) if ($connection) {
            try { $connection->close(); } catch (Throwable $error) { $diagnostic .= ' Close: ' . $error->getMessage(); }
        }
    }
    if ($diagnostic !== '') $failed = true;
    echo "@YDB_CONTRACT\t$case\t" . ($diagnostic === '' ? 'passed' : 'failed') . "\t" . bin2hex($diagnostic) . "\n";
}
exit($failed ? 1 : 0);
