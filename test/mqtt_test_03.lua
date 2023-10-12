local r1 = M.mqtt.publish("mqtt_local")
local r2 = M.mqtt.publish("mqtt_local", "mink/DEBUG_UUID/1/cmd", false)
local r3 = M.mqtt.publish("mqtt_local_XX", "mink/DEBUG_UUID/1/AAA", "XX")

if r1 or r2 or r3  then
    return "ERR"
else
    r1 = M.mqtt.publish("mqtt_local", "mink/DEBUG_UUID/1/cmd", "test_dummy_data_02")
    if r1 then
        return "OK"
    else
        return "ERR"
    end
end
