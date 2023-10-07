local d, r = M.signal("TEST_EVENT_01_ADMIN", "", "{\"flags\": 0}")
if r ~= 2 then
    return "ERR"
else
    d, r = M.signal("TEST_EVENT_01_ADMIN", "", "{\"flags\": 1}")
    return d
end
