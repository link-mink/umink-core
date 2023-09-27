-- dummy generic CMD CALL
cmd = {
    "CMD_LUA_CALL",
    "test_data_01",
    "test_data_02",
    "test_data_03"
}
-- run and expect result
local res = M.cmd_call(cmd)
-- expect result of 4 table items
if res == nil or #res < 4 then
    return nil
else
    return res[4].test_res_key
end
