-- incremental
M.perf_inc("test_05_c1")
M.perf_inc("test_05_c1")
-- try missing counter
M.perf_inc()
M.perf_inc(1000)
M.perf_inc(false)
-- gauge
M.perf_set("test_05_c2", 999)
M.perf_set("test_05_c2", 998)
-- the following should fail
-- and remain at 1
M.perf_set("test_05_c1", 1)
-- match
local d = M.perf_match("test_05_c*")
-- sort keys
local rt = {}
local res = ""
for k, v in pairs(d) do
    table.insert(rt, k)
end
table.sort(rt)
-- return string
return rt[1] .. rt[2]
