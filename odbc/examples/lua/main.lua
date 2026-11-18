local odbc = require('luasql.odbc')
local connection_string = assert(arg[1] or os.getenv('ODBC_CONNECTION_STRING'),
                                 'pass a connection string or set ODBC_CONNECTION_STRING')
local environment = assert(odbc.odbc())
local connection = assert(environment:connect(connection_string))
local cursor = assert(connection:execute("SELECT CAST(42 AS Double) AS answer, 'Hello YDB' AS message"))
local row = {}
while cursor:fetch(row, 'n') do
  io.write(string.format('%g\t%s\n', row[1], row[2]))
end
assert(connection:close())
assert(environment:close())
