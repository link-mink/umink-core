local args = M.get_args()[1][1]
local res = ""
if args == "set" then
    M.db_set("test_db", "test_key", "test_value")
else
    res = M.db_get("test_db", "test_key")
end
return res
