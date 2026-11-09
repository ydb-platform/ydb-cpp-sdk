local env = assert(require("luasql.odbc").odbc())
local conn = assert(env:connect(arg[1]))
local cur = assert(conn:execute([[SELECT
  CAST('b' AS VARCHAR(10)), CAST('c' AS VARCHAR(10)),
  CAST(NULL AS VARCHAR(10))]]))
print("strings:", cur:fetch())
assert(cur:close())
for _, value in ipairs {"4294967296", "-4294967297"} do
  cur = assert(conn:execute("SELECT CAST(" .. value .. " AS BIGINT)"))
  print(value .. ":", cur:fetch())
  assert(cur:close())
end
local columns = {"CAST('x' AS VARCHAR(10))"}
for i=2,40 do columns[i] = "CAST(NULL AS VARCHAR(10))" end
cur = assert(conn:execute("SELECT " .. table.concat(columns, ",")))
print("wide NULL row:", cur:fetch())
assert(cur:close())
assert(conn:close())
assert(env:close())
