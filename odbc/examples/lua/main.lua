local odbc = require('luasql.odbc')
local connection_string = assert(arg[1] or os.getenv('ODBC_CONNECTION_STRING'),
                                 'pass a connection string or set ODBC_CONNECTION_STRING')
local environment = assert(odbc.odbc())
local connection = assert(environment:connect(connection_string))
local table_name = 'lua_odbc_example_' .. os.time() .. '_' .. tostring({}):match('(%x+)$')
local created, reader, insert, query, cursor, other

local function price(value)
  return value == nil and 'NULL' or string.format('%g', value)
end

local ok, message = xpcall(function()
  assert(connection:execute('CREATE TABLE ' .. table_name ..
    ' (sku Utf8 NOT NULL, name Utf8, price Double, PRIMARY KEY (sku))'))
  created = true
  insert = assert(connection:prepare('INSERT INTO ' .. table_name ..
    ' (sku, name, price) VALUES (CAST(? AS Utf8), CAST(? AS Utf8), CAST(? AS Double))'))
  print('inserted', assert(insert:execute('tea', 'Чай', 2.5)))
  print('inserted', assert(insert:execute('coffee', 'Café', 4.25)))
  print('inserted', assert(insert:execute('water', 'Water', nil)))

  cursor = assert(connection:execute('SELECT sku, name, price FROM ' .. table_name .. ' ORDER BY sku'))
  print('columns', table.concat(cursor:getcolnames(), '\t'))
  print('types', table.concat(cursor:getcoltypes(), '\t'))
  local row = {}
  while cursor:fetch(row, 'n') do
    print(row[1], row[2], price(row[3]))
  end
  cursor = nil -- LuaSQL closes a cursor after its last row.
  print('updated', assert(connection:execute('UPDATE ' .. table_name .. " SET price = 4.5 WHERE sku = 'coffee'")))
  print('deleted', assert(connection:execute('DELETE FROM ' .. table_name .. " WHERE sku = 'missing'")))

  -- LuaSQL loses SQLSTATE and mishandles diagnostics on commit errors.
  -- Reliable retries need those binding defects fixed first.
  reader = assert(environment:connect(connection_string))
  assert(connection:setautocommit(false))
  assert(connection:execute('UPDATE ' .. table_name .. " SET price = 9.5 WHERE sku = 'tea'"))
  assert(connection:rollback())
  cursor = assert(reader:execute('SELECT price FROM ' .. table_name .. " WHERE sku = 'tea'"))
  print('after rollback', price(cursor:fetch()))
  assert(cursor:close())
  cursor = nil
  assert(connection:execute('UPDATE ' .. table_name .. " SET price = 3.5 WHERE sku = 'tea'"))
  assert(connection:commit())
  assert(connection:setautocommit(true))
  cursor = assert(reader:execute('SELECT price FROM ' .. table_name .. " WHERE sku = 'tea'"))
  print('after commit', price(cursor:fetch()))
  assert(cursor:close())
  cursor = nil

  -- Discard unread rows before executing the same prepared statement again.
  query = assert(connection:prepare('SELECT sku, price FROM ' .. table_name ..
    ' WHERE price > CAST(? AS Double) ORDER BY sku'))
  cursor = assert(query:execute(0))
  local sku, amount = cursor:fetch()
  print('first', sku, price(amount))
  other = assert(reader:execute('SELECT name FROM ' .. table_name .. " WHERE sku = 'tea'"))
  print('independent reader', other:fetch())
  assert(other:close())
  other = nil
  assert(cursor:close())
  cursor = assert(query:execute(4))
  while cursor:fetch(row, 'n') do
    print('rebound', row[1], price(row[2]))
  end
  cursor = nil
  print('deleted existing', assert(connection:execute('DELETE FROM ' .. table_name .. " WHERE sku = 'water'")))
  cursor = assert(connection:execute('SELECT CAST(COUNT(*) AS Double) FROM ' .. table_name))
  print('remaining', price(cursor:fetch()))
  assert(cursor:close())
  cursor = nil
end, debug.traceback)

-- Release handles on both the successful and error paths.
if other then pcall(function() other:close() end) end
if cursor then pcall(function() cursor:close() end) end
if query then assert(query:close()) end
if insert then assert(insert:close()) end
if reader then assert(reader:close()) end
assert(connection:rollback())
assert(connection:setautocommit(true))
if created then assert(connection:execute('DROP TABLE ' .. table_name)) end
assert(connection:close())
assert(environment:close())
if not ok then error(message, 0) end
