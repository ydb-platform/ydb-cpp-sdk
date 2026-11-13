local driver = require('luasql.odbc')
local mode, connection = arg[1], arg[2]
assert(mode == 'dsn' or mode == 'connection_string', 'invalid connection mode')

local function check(ok, message)
  assert(ok, message)
end
local function finally(body, cleanup)
  local ok, result = pcall(body)
  local cleaned, problem = pcall(cleanup)
  if not ok then error(result, 0) end
  if not cleaned then error(problem, 0) end
  return result
end
local function using(object, body)
  return finally(function() return body(object) end, function() assert(object:close()) end)
end
local function with_connection(body)
  return using(assert(driver.odbc()), function(env)
    return using(assert(env:connect(connection)), body)
  end)
end
local function execute(con, sql)
  return assert(con:execute(sql))
end
local function scalar(con, sql)
  return using(execute(con, sql), function(cur) return cur:fetch() end)
end
local function with_table(con, name, body)
  local table_name = 'contract_lua_' .. name
  execute(con, 'DROP TABLE IF EXISTS ' .. table_name)
  execute(con, 'CREATE TABLE ' .. table_name .. ' (id Int32, value Utf8, PRIMARY KEY (id))')
  return finally(function() body(table_name) end,
                 function() execute(con, 'DROP TABLE IF EXISTS ' .. table_name) end)
end

local cases = {
  {'connection', function()
    with_connection(function(con)
      check(scalar(con, "SELECT 'connected'u AS value") == 'connected', 'connection query differs')
    end)
  end},
  {'binding', function()
    with_connection(function(con)
      using(assert(con:prepare('SELECT CAST(CAST(? AS Int32) AS Utf8) AS id, CAST(? AS Utf8) AS value, ' ..
                              "CASE WHEN CAST(? AS Double) IS NULL THEN 'null'u ELSE 'non-null'u END AS missing")), function(stmt)
        using(assert(stmt:execute(7, 'Привет 🌍', nil)), function(cur)
          local row = assert(cur:fetch({}, 'n'))
          check(row[1] == '7' and row[2] == 'Привет 🌍' and row[3] == 'null', 'bound values differ')
        end)
      end)
    end)
  end},
  {'metadata', function()
    with_connection(function(con)
      with_table(con, 'metadata', function(table_name)
        using(execute(con, 'SELECT id, value FROM ' .. table_name), function(cur)
          local names, types = cur:getcolnames(), cur:getcoltypes()
          check(#names == 2 and names[1]:lower() == 'id' and names[2]:lower() == 'value',
                'result column names differ')
          check(types[1] == 'integer' and types[2] == 'string', 'result column types differ')
        end)
      end)
    end)
  end},
  {'row_count', function()
    with_connection(function(con)
      with_table(con, 'row_count', function(table_name)
        check(execute(con, 'UPSERT INTO ' .. table_name ..
          " (id, value) VALUES (1, 'one'u), (2, 'two'u), (3, 'three'u)") == 3,
          'insert affected-row count differs')
        check(execute(con, 'UPDATE ' .. table_name .. " SET value = 'updated'u WHERE id <= 2") == 2,
              'update affected-row count differs')
        check(execute(con, 'DELETE FROM ' .. table_name .. ' WHERE id = 3') == 1,
              'delete affected-row count differs')
        check(execute(con, 'DELETE FROM ' .. table_name .. ' WHERE id = 99') == 0,
              'no-op affected-row count differs')
      end)
    end)
  end},
  {'transactions', function()
    with_connection(function(con)
      with_table(con, 'transactions', function(table_name)
        execute(con, 'UPSERT INTO ' .. table_name .. " (id, value) VALUES (1, 'seed'u)")
        assert(con:setautocommit(false))
        finally(function()
          execute(con, 'UPDATE ' .. table_name .. " SET value = 'discarded'u WHERE id = 1")
          assert(con:rollback())
          check(scalar(con, 'SELECT value FROM ' .. table_name .. ' WHERE id = 1') == 'seed',
                'rollback changed stored value')
          assert(con:commit())
          execute(con, 'UPDATE ' .. table_name .. " SET value = 'committed'u WHERE id = 1")
          assert(con:commit())
          check(scalar(con, 'SELECT value FROM ' .. table_name .. ' WHERE id = 1') == 'committed',
                'commit did not persist value')
        end, function() assert(con:rollback()); assert(con:setautocommit(true)) end)
      end)
    end)
  end},
  {'independent_connections', function()
    with_connection(function(first)
      with_table(first, 'independent_connections', function(table_name)
        with_connection(function(second)
          assert(first:setautocommit(false))
          finally(function()
            execute(first, 'UPSERT INTO ' .. table_name .. " (id, value) VALUES (1, 'pending'u)")
            check(scalar(second, "SELECT CASE WHEN COUNT(*) = 0 THEN 'absent'u ELSE 'visible'u END FROM " ..
                         table_name) == 'absent', 'second connection saw an uncommitted write')
            assert(second:setautocommit(false))
            execute(second, 'UPSERT INTO ' .. table_name .. " (id, value) VALUES (2, 'committed'u)")
            assert(second:commit())
            assert(first:rollback())
            assert(first:setautocommit(true))
            assert(second:setautocommit(true))
            check(scalar(first, 'SELECT value FROM ' .. table_name) == 'committed' and
                  scalar(first, "SELECT CASE WHEN COUNT(*) = 1 THEN 'one'u ELSE 'many'u END FROM " ..
                         table_name) == 'one', "rollback on first connection changed second connection's commit")
          end, function()
            assert(first:rollback()); assert(second:rollback())
            assert(first:setautocommit(true)); assert(second:setautocommit(true))
          end)
        end)
      end)
    end)
  end},
  {'cleanup', function()
    with_connection(function(con)
      local cur = execute(con, "SELECT 'first'u AS value UNION ALL SELECT 'second'u AS value ORDER BY value")
      using(cur, function(result) check(result:fetch() == 'first', 'initial fetch failed') end)
      check(cur:close() == false, 'closed cursor remained open')
      check(scalar(con, "SELECT 'reused'u AS value") == 'reused', 'connection reuse after cleanup failed')
    end)
  end},
}

local failed = false
for _, case in ipairs(cases) do
  local ok, diagnostic = xpcall(case[2], debug.traceback)
  if ok then diagnostic = '' else failed = true end
  local hex = diagnostic:gsub('.', function(byte) return string.format('%02x', byte:byte()) end)
  io.write('@YDB_CONTRACT\t', case[1], '\t', ok and 'passed' or 'failed', '\t', hex, '\n')
end
os.exit(failed and 1 or 0)
