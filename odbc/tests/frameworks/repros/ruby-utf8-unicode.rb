require 'odbc_utf8'
connection = ODBC::Database.new.drvconnect(ARGV.fetch(0))
statement = connection.prepare('SELECT ? AS value')
expected = "\u{1f30d}"
statement.execute(expected)
actual = statement.fetch.first
puts "expected=#{expected.b.unpack1('H*')} actual=#{actual.b.unpack1('H*')}"
statement.drop
connection.disconnect
exit(actual.b == expected.b ? 0 : 1)
