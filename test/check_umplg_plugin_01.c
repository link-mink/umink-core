/*
 *               _____  ____ __
 *   __ ____ _  /  _/ |/ / //_/
 *  / // /  ' \_/ //    / ,<
 *  \_,_/_/_/_/___/_/|_/_/|_|
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include <stdlib.h>
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <string.h>
#include <cmocka_tests.h>
#include <umink_pkg_config.h>
#include <umink_plugin.h>

/*************/
/* Plugin ID */
/*************/
static const char *PLG_ID = "test_plugin_01";

/**********************************************/
/* list of command implemented by this plugin */
/**********************************************/
int COMMANDS[] = {
    // dummy command id
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
    return 0;
}

/*************************/
/* local command handler */
/*************************/
int
run_local(umplg_mngr_t *pm, umplgd_t *pd, int cmd_id, umplg_idata_t *data)
{
    // id = 1000
    if (cmd_id == 1000) {
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
