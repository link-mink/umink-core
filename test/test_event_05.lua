local a,r = M.signal()
-- unknwon signal
if r ~= 1 then
    return "ERR"
else
    -- invalid type
    a, r = M.signal(nil)
    if r~= 7 then
    else
        a, r = M.signal("TEST_EVENT_01")
        return a
    end
end
