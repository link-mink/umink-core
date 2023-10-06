local args = M.get_args()[1][1]
local d1 = M.test.test_method_01()
local d2 = M.test.test_method_02()
local d3 = M.test.test_method_02("arg_test")
local res = d1 .. (d2 == nil and "nil" or "ERR") ..d3
return res
