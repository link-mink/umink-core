/*
 *               _____  ____ __
 *   __ ____ _  /  _/ |/ / //_/
 *  / // /  ' \_/ //    / ,<
 *  \_,_/_/_/_/___/_/|_/_/|_|
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include <config.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <string.h>
#include <cmocka_tests.h>
#include <umink_pkg_config.h>
#include <umink_plugin.h>
#include <luaconf.h>
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

/*************/
/* Plugin ID */
/*************/
static const char *PLG_ID = "test_plugin_01";

/**********************************************/
/* list of command implemented by this plugin */
/**********************************************/
int COMMANDS[] = {
    // predefined cmd id (unused, only for testing)
    CMD_LUA_CALL,
    // dummy command is
    1000,
    2000,
    // end of list marker
    -1
};

/*********************/
/* terminate handler */
/*********************/
int
terminate(umplg_mngr_t *pm, umplgd_t *pd, int phase)
{
    return 0;
}

static int
mink_lua_test_method_01(lua_State *L)
{
    lua_pushstring(L, "test_method_return_string_01");
    return 1;
}

static int
mink_lua_test_method_02(lua_State *L)
{
    if (lua_gettop(L) < 1) {
        return 0;
    }
    const char *arg = lua_tostring(L, 1);
    lua_pushstring(L, arg);
    return 1;
}


static const struct luaL_Reg test_lualib[] = {
    { "test_method_01", &mink_lua_test_method_01 },
    { "test_method_02", &mink_lua_test_method_02 },
    { NULL, NULL }
};

static void
init_test_lua_module(lua_State *L)
{
    luaL_newlib(L, test_lualib);
}

/********************************************/
/* test module create (signal init handler) */
/********************************************/
static int
test_module_sig_run(umplg_sh_t *shd,
                    umplg_data_std_t *d_in,
                    char **d_out,
                    size_t *out_sz,
                    void *args)
{

    // get lua state (assume args != NULL)
    lua_State *L = args;
    // get M module from globals
    lua_getglobal(L, "M");
    // add test sub-module
    lua_pushstring(L, "test");
    init_test_lua_module(L);
    // add test module table to M table
    lua_settable(L, -3);
    // remove M table from stack
    lua_pop(L, 1);

    // success
    return 0;
}


/****************/
/* init handler */
/****************/
#ifdef FAIL_TESTS
static int
#else
int
#endif
init(umplg_mngr_t *pm, umplgd_t *pd)
{
    // create signal handler for creating MQTT module
    // when per-thread Lua state creates the M module
    umplg_sh_t *sh = calloc(1, sizeof(umplg_sh_t));
    sh->id = strdup("@init_lua_sub_module:test");
    sh->run = &test_module_sig_run;
    sh->running = false;

    // register signal
    umplg_reg_signal(pm, sh);

    return 0;
}

/*************************/
/* local command handler */
/*************************/
int
run_local(umplg_mngr_t *pm, umplgd_t *pd, int cmd_id, umplg_idata_t *data)
{
    // id = CMD_LUA_CALL
    if (cmd_id == CMD_LUA_CALL) {
        // get data map
        umplg_data_std_t *dmap = data->data;
        // get data, 3 input strings expected
        if (utarray_len(dmap->items) < 3) {
            return -1;
        }
        umplg_data_std_items_t *item_01 = utarray_eltptr(dmap->items, 0);
        umplg_data_std_items_t *item_02 = utarray_eltptr(dmap->items, 1);
        umplg_data_std_items_t *item_03 = utarray_eltptr(dmap->items, 2);
        if (strcmp(item_01->table->value, "test_data_01") != 0 ||
            strcmp(item_02->table->value, "test_data_02") != 0 ||
            strcmp(item_03->table->value, "test_data_03") != 0) {
            return -1;
        }
        // add result
        umplg_data_std_items_t items = { .table = NULL };
        umplg_data_std_item_t item_res = { .name = "test_res_key",
                                           .value = "test_res_value" };
        umplg_stdd_item_add(&items, &item_res);
        umplg_stdd_items_add(dmap, &items);
        HASH_CLEAR(hh, items.table);

        return 0;

        // id = 1000
    } else if (cmd_id == 1000) {
        return 0;

        // id = 2000
    } else if (cmd_id == 2000) {
        // get data map
        umplg_data_std_t *dmap = data->data;
        // check param count
        if (data == NULL || utarray_len(dmap->items) < 2) {
            return -1;
        }
        // get dummy "test_key_01" = "test_value_01"
        umplg_data_std_items_t *item_01 = utarray_eltptr(dmap->items, 0);
        if (strcmp(item_01->table->name, "test_key_01") != 0 ||
            strcmp(item_01->table->value, "test_value_01") != 0) {
            return -1;
        }

        // get dummy "test_key_02" = "test_value_02"
        umplg_data_std_items_t *item_02 = utarray_eltptr(dmap->items, 1);
        if (strcmp(item_02->table->name, "test_key_02") != 0 ||
            strcmp(item_02->table->value, "test_value_02") != 0) {
            return -1;
        }

        // add result
        umplg_data_std_items_t items = { .table = NULL };
        umplg_data_std_item_t item_res = { .name = "test_res_key",
                                           .value = "test_res_value" };
        umplg_stdd_item_add(&items, &item_res);
        umplg_stdd_items_add(dmap, &items);
        HASH_CLEAR(hh, items.table);

        return 0;
    }

    return -1;
}

/*******************/
/* command handler */
/*******************/
int
run(umplg_mngr_t *pm, umplgd_t *pd, int cmd_id, umplg_idata_t *data)
{
    // not used
    return 0;
}
