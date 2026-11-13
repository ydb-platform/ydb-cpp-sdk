VARIANT = ARGV.shift
require VARIANT
MODE, CONNECTION = ARGV
raise 'invalid connection mode' unless %w[dsn connection_string].include?(MODE)

def check(ok, message)
  raise message unless ok
end

def connect
  MODE == 'dsn' ? ODBC.connect(CONNECTION) : ODBC::Database.new.drvconnect(CONNECTION)
end

def with_connection
  con = connect
  yield con
ensure
  con.disconnect if con&.connected?
end

def with_statement(stmt)
  yield stmt
ensure
  stmt&.drop
end

def with_table(con, name)
  table = "contract_ruby_#{VARIANT}_#{name}"
  con.do("DROP TABLE IF EXISTS #{table}")
  con.do("CREATE TABLE #{table} (id Int32, value Utf8, PRIMARY KEY (id))")
  begin
    yield table
  ensure
    con.do("DROP TABLE IF EXISTS #{table}")
  end
end

def scalar(con, sql)
  with_statement(con.run(sql)) { |stmt| stmt.fetch.first }
end

cases = {
  'connection' => proc do
    with_connection do |con|
      check(con.connected?, 'connection is invalid')
      check(scalar(con, "SELECT 'connected'u AS value") == 'connected', 'connection query differs')
    end
  end,
  'binding' => proc do
    with_connection do |con|
      with_statement(con.prepare('SELECT CAST(? AS Int32) AS id, CAST(? AS Utf8) AS value, ' \
                                 'CAST(? AS Utf8) AS missing')) do |stmt|
        unicode = 'Привет 🌍'
        stmt.execute(7, unicode, nil)
        row = stmt.fetch
        check(row[0] == 7 && row[1].b == unicode.b && row[2].nil?,
              "bound values differ: id=#{row[0].inspect}, text=#{row[1].b.unpack1('H*')}, null=#{row[2].nil?}")
        check(stmt.fetch.nil?, 'unexpected bound result row')
      end
    end
  end,
  'metadata' => proc do
    with_connection do |con|
      with_table(con, 'metadata') do |table|
        with_statement(con.tables(table)) do |stmt|
          check(stmt.fetch_all.any? { |row| row[2] == table }, 'table missing from catalog')
        end
        with_statement(con.columns(table)) do |stmt|
          check(stmt.fetch_all.map { |row| row[3].downcase } == %w[id value], 'catalog columns differ')
        end
        with_statement(con.run("SELECT id, value FROM #{table}")) do |stmt|
          check(stmt.columns(true).map { |column| column.name.downcase } == %w[id value],
                'result columns differ')
        end
      end
    end
  end,
  'row_count' => proc do
    with_connection do |con|
      with_table(con, 'row_count') do |table|
        check(con.do("UPSERT INTO #{table} (id, value) VALUES (1, 'one'u), (2, 'two'u), (3, 'three'u)") == 3,
              'insert affected-row count differs')
        check(con.do("UPDATE #{table} SET value = 'updated'u WHERE id <= 2") == 2,
              'update affected-row count differs')
        check(con.do("DELETE FROM #{table} WHERE id = 3") == 1, 'delete affected-row count differs')
        check(con.do("DELETE FROM #{table} WHERE id = 99") == 0, 'no-op affected-row count differs')
      end
    end
  end,
  'transactions' => proc do
    with_connection do |con|
      with_table(con, 'transactions') do |table|
        con.do("UPSERT INTO #{table} (id, value) VALUES (1, 'seed'u)")
        con.autocommit = false
        begin
          con.do("UPDATE #{table} SET value = 'discarded'u WHERE id = 1")
          con.rollback
          check(scalar(con, "SELECT value FROM #{table} WHERE id = 1") == 'seed',
                'rollback changed stored value')
          con.commit
          con.do("UPDATE #{table} SET value = 'committed'u WHERE id = 1")
          con.commit
          check(scalar(con, "SELECT value FROM #{table} WHERE id = 1") == 'committed',
                'commit did not persist value')
        ensure
          con.rollback
          con.autocommit = true
        end
      end
    end
  end,
  'independent_connections' => proc do
    with_connection do |first|
      with_table(first, 'independent_connections') do |table|
        with_connection do |second|
          first.autocommit = false
          begin
            first.do("UPSERT INTO #{table} (id, value) VALUES (1, 'pending'u)")
            with_statement(second.run("SELECT value FROM #{table}")) do |stmt|
              check(stmt.fetch.nil?, 'second connection saw an uncommitted write')
            end
            second.autocommit = false
            second.do("UPSERT INTO #{table} (id, value) VALUES (2, 'committed'u)")
            second.commit
            first.rollback
            first.autocommit = second.autocommit = true
            with_statement(first.run("SELECT value FROM #{table}")) do |stmt|
              check(stmt.fetch_all == [['committed']],
                    "rollback on first connection changed second connection's commit")
            end
          ensure
            first.rollback
            second.rollback
            first.autocommit = second.autocommit = true
          end
        end
      end
    end
  end,
  'cleanup' => proc do
    with_connection do |con|
      with_statement(con.run('SELECT 1 AS value UNION ALL SELECT 2 AS value ORDER BY value')) do |stmt|
        check(stmt.fetch == [1], 'initial fetch failed')
      end
      check(scalar(con, "SELECT 'reused'u AS value") == 'reused', 'connection reuse after cleanup failed')
      con.disconnect
      check(!con.connected?, 'disconnected connection is still valid')
    end
  end,
  'multiple_results' => proc do
    with_connection do |con|
      with_statement(con.run('SELECT 1 AS first UNION ALL SELECT 2 AS first ORDER BY first; ' \
                             "SELECT 'second'u AS label, 22 AS value; SELECT 33 AS last")) do |stmt|
        check(stmt.columns(true).map(&:name) == ['first'], 'first result schema differs')
        check(stmt.fetch == [1], 'first result row differs')
        check(stmt.more_results, 'second result missing')
        check(stmt.columns(true).map(&:name) == %w[label value], 'second result schema differs')
        check(stmt.fetch == ['second', 22], 'second result row differs')
        check(stmt.more_results, 'third result missing')
        check(stmt.columns(true).map(&:name) == ['last'], 'third result schema differs')
        check(stmt.fetch == [33] && stmt.fetch.nil?, 'third result rows differ')
        check(!stmt.more_results && !stmt.more_results, 'unexpected result after end')
        stmt.run('SELECT 44 AS discarded; SELECT 55 AS pending')
        stmt.close
        check(!stmt.more_results, 'closed batch retained pending results')
        stmt.run('SELECT 66 AS discarded; SELECT 77 AS pending')
        stmt.cancel
        check(!stmt.more_results, 'canceled batch retained pending results')
        stmt.run("SELECT 'reused'u AS value")
        check(stmt.fetch == ['reused'] && !stmt.more_results, 'statement reuse after batch failed')
      end
    end
  end,
  'concurrency' => proc do
    gate = Queue.new
    threads = 2.times.map do |index|
      Thread.new do
        Thread.current.report_on_exception = false
        gate.pop
        with_connection do |con|
          8.times do |iteration|
            value = "thread_#{index}_#{iteration}"
            check(scalar(con, "SELECT '#{value}'u AS value") == value, 'concurrent result differs')
          end
        end
      end
    end
    2.times { gate << true }
    errors = threads.filter_map do |thread|
      begin
        thread.value
        nil
      rescue StandardError => error
        error
      end
    end
    raise errors.first unless errors.empty?
  end
}

failed = false
cases.each do |name, test|
  begin
    test.call
    status, diagnostic = 'passed', ''
  rescue StandardError => error
    failed = true
    status, diagnostic = 'failed', "#{error.class}: #{error.message}"
    warn error.full_message(highlight: false)
  end
  puts ['@YDB_CONTRACT', name, status, diagnostic.encode('UTF-8').unpack1('H*')].join("\t")
end
exit(failed ? 1 : 0)
