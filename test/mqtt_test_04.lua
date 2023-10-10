local cmd = {
    "CMD_MQTT_BINARY_UPLOAD",
    "mqtt_local",
    1024
}

local cmd2 = {
    "CMD_MQTT_BINARY_UPLOAD",
    "mqtt_local",
    1024
}

-- run
local d = M.cmd_call(cmd)
local d2 = M.cmd_call(cmd)
-- notify upload script to start
-- sending 256 byte chunks
os.execute("echo \"" .. d[3].file_uuid .. "\" > /tmp/tmp_file_uuid")
os.execute("echo \"" .. d2[3].file_uuid .. "\" > /tmp/tmp_file_uuid_timeout")
return d[3].file_uuid .. ":" .. d2[3].file_uuid
