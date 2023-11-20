local args = M.get_args()[1][1]
local d, r = M.signal(args)
return r .. ":" .. d
