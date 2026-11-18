require ENV.fetch('RUBY_ODBC_MODULE', 'odbc')

connection_string = ARGV.first || ENV.fetch('ODBC_CONNECTION_STRING')
connection = ODBC::Database.new.drvconnect(connection_string)
table = "ruby_odbc_example_#{Process.pid}"
created = false
begin
  connection.do("CREATE TABLE #{table} (id Int32 NOT NULL, name Utf8, note Utf8, quantity Int32, PRIMARY KEY (id))")
  created = true
  insert = connection.prepare("INSERT INTO #{table} (id, name, note, quantity) VALUES (Unwrap(CAST(? AS Int32)), CAST(? AS Utf8), CAST(? AS Utf8), CAST(? AS Int32))")
  [[1, 'Чай', nil, 10], [2, 'Café', 'fresh', 20], [3, 'Water', 'still', 30]].each do |values|
    insert.execute(*values)
    puts "inserted\t#{insert.nrows}"
  end

  connection.run("SELECT id, name, note, quantity FROM #{table} ORDER BY id") do |rows|
    puts ['columns', *rows.columns(true).map(&:name)].join("\t")
    rows.each { |row| puts row.map { |value| value.nil? ? 'NULL' : value }.join("\t") }
  end
  connection.tables(table) { |tables| puts "matching tables\t#{tables.fetch_all.length}" }
  connection.columns(table) do |columns|
    puts ['schema', *columns.fetch_all.map { |column| column[3] }].join("\t")
  end
  puts "updated\t#{connection.do("UPDATE #{table} SET quantity = 21 WHERE id = 2")}"
  puts "deleted\t#{connection.do("DELETE FROM #{table} WHERE id = 999")}"

  # The reader has its own connection and sees only committed changes.
  reader = ODBC::Database.new.drvconnect(connection_string)
  connection.autocommit = false
  connection.do("UPDATE #{table} SET quantity = 99 WHERE id = 1")
  connection.rollback
  reader.run("SELECT quantity FROM #{table} WHERE id = 1") do |rows|
    puts "after rollback\t#{rows.fetch.first}"
  end
  connection.do("UPDATE #{table} SET quantity = 11 WHERE id = 1")
  connection.commit
  connection.autocommit = true
  reader.run("SELECT quantity FROM #{table} WHERE id = 1") do |rows|
    puts "after commit\t#{rows.fetch.first}"
  end

  # Retry the entire transaction, including commit, only on serialization failure.
  attempts = 0
  connection.autocommit = false
  begin
    attempts += 1
    quantity = connection.run("SELECT quantity FROM #{table} WHERE id = 1") { |rows| rows.fetch.first }
    # A competing connection creates one real serialization conflict.
    reader.do("UPDATE #{table} SET quantity = quantity + 1 WHERE id = 1") if attempts == 1
    connection.do("UPDATE #{table} SET quantity = #{quantity + 5} WHERE id = 1")
    connection.commit
  rescue ODBC::Error => error
    connection.rollback
    raise unless error.message.match?(/\A40001\b/) && attempts < 3
    retry
  end
  connection.autocommit = true
  puts "transaction attempts\t#{attempts}"
  reader.run("SELECT quantity FROM #{table} WHERE id = 1") { |rows| puts "after retry\t#{rows.fetch.first}" }

  # Executing again discards unread rows and binds a new value on the statement.
  query = connection.prepare("SELECT id, name FROM #{table} WHERE id >= CAST(? AS Int32) ORDER BY id")
  query.execute(1)
  puts ['first', *query.fetch].join("\t")
  reader.run("SELECT COUNT(*) AS total FROM #{table}") do |rows|
    puts "independent reader\t#{rows.fetch.first}"
  end
  query.execute(2)
  query.each { |row| puts ['rebound', *row].join("\t") }

  connection.run("SELECT name FROM #{table} WHERE id = 1; SELECT quantity FROM #{table} WHERE id = 2") do |results|
    loop do
      puts ['result columns', *results.columns(true).map(&:name)].join("\t")
      results.each { |row| puts ['result', *row].join("\t") }
      break unless results.more_results
    end
  end
  puts "deleted existing\t#{connection.do("DELETE FROM #{table} WHERE id = 3")}"
  connection.run("SELECT COUNT(*) FROM #{table}") { |rows| puts "remaining\t#{rows.fetch.first}" }
ensure
  query&.drop
  insert&.drop
  reader&.disconnect
  if connection.connected?
    connection.rollback unless connection.autocommit
    connection.autocommit = true
    connection.do("DROP TABLE #{table}") if created
    connection.disconnect
  end
end
