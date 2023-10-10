cmd = {
    "CMD_MQTT_BINARY_UPLOAD",
    "mqtt_local",
    1024
}
-- run
local d = M.cmd_call(cmd)
-- notify upload script to start
-- sending 256 byte chunks
os.execute("echo \"" .. d[3].file_uuid .. "\" > /tmp/tmp_file_uuid")
return d[3].file_uuid
