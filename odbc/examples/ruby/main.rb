require ENV.fetch('RUBY_ODBC_MODULE', 'odbc')

connection_string = ARGV.first || ENV.fetch('ODBC_CONNECTION_STRING')
connection = ODBC::Database.new.drvconnect(connection_string)
begin
  statement = connection.prepare('SELECT CAST(? AS Int32) AS answer, CAST(? AS Utf8) AS message')
  statement.execute(42, 'Hello YDB')
  while (row = statement.fetch)
    puts row.join("\t")
  end
ensure
  statement&.drop
  connection.disconnect
end
