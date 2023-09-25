local args = M.get_args()[1][1]
local d = M.signal("TEST_EVENT_02", "event_02_args")
return args .. d
