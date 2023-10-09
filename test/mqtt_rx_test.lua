local data = M.get_args()
local T = data[1].mqtt_topic
local P = data[1].mqtt_payload
local C = data[1].mqtt_connection
local val = T .. P .. C
M.db_set("test_db", "test_key",  val)
return val
