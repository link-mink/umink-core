/*
 *               _____  ____ __
 *   __ ____ _  /  _/ |/ / //_/
 *  / // /  ' \_/ //    / ,<
 *  \_,_/_/_/_/___/_/|_/_/|_|
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include <umink_pkg_config.h>
#include <umink_plugin.h>
#include <string.h>
#include <luaconf.h>
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <umdaemon.h>
#include <umdb.h>
#include <json_object.h>
#include <json_tokener.h>
#include <MQTTAsync.h>

extern struct mqtt_conn_mngr *mqtt_mngr;
struct mqtt_conn_d *mqtt_mngr_get_conn(struct mqtt_conn_mngr *m,
                                       const char *name);
int mqtt_mngr_send(struct mqtt_conn_mngr *m,
                   struct mqtt_conn_d *c,
                   const char *t,
                   bool retain,
                   void *d,
                   size_t dsz);

int
mink_lua_mqtt_pub(lua_State *L)
{
    // min 3 args: conn name, topic, data
    if (lua_gettop(L) < 3) {
        lua_pushboolean(L, false);
        return 1;
    }
    // do not retain by default
    bool retain = false;

    // 4th arg (boolean) = mqtt retain flag
    if (lua_gettop(L) > 3 && lua_isboolean(L, 4)) {
        retain = lua_toboolean(L, 4);
    }

    // data types check (all strings)
    if (!(lua_isstring(L, 1) && lua_isstring(L, 2) && lua_isstring(L, 3))) {
        lua_pushboolean(L, false);
        return 1;
    }

    // get args
    const char *c = lua_tostring(L, 1);
    const char *t = lua_tostring(L, 2);
    const char *d = lua_tostring(L, 3);

    // get connection
    struct mqtt_conn_d *conn = mqtt_mngr_get_conn(mqtt_mngr, c);
    if (conn == NULL) {
        lua_pushboolean(L, false);
        return 1;
    }

    // send
    int r = mqtt_mngr_send(mqtt_mngr, conn, t, retain, (void *)d, strlen(d));
    if (r != MQTTASYNC_SUCCESS) {
        lua_pushboolean(L, false);
        return 1;
    }

    // success
    lua_pushboolean(L, true);
    return 1;
}
