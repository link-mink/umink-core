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
#include <cmocka.h>
#include <string.h>
#include <stdio.h>
#include <umdaemon.h>
#include <umink_plugin.h>

static void
umplg_load_test(void **state)
{
    umdaemon_t *umd = umd_create("test_id", "test_type");

    // create plg manager
    umplg_mngr_t *m = umplg_new_mngr();

    // load dummy plugin (success)
    umplgd_t *p = umplg_load(m, ".libs/check_umplg_plugin_01.so");
    assert_non_null(p);

    // load dummy plugin (failure due to missing init)
    umplgd_t *p2 = umplg_load(m, ".libs/check_umplg_plugin_02.so");
    assert_null(p2);

    // free
    umplg_free_mngr(m);
    umd_destroy(umd);
}

static void
umplg_run_test_01(void **state)
{
    umdaemon_t *umd = umd_create("test_id", "test_type");

    // create plg manager
    umplg_mngr_t *m = umplg_new_mngr();

    // load dummy plugin (success)
    umplgd_t *p = umplg_load(m, ".libs/check_umplg_plugin_01.so");
    assert_non_null(p);

    // call dummy method id = 1000

    // input/output buffer
    umplg_data_std_t d = { .items = NULL };
    umplg_stdd_init(&d);
    // plugin inpud data wrapper
    umplg_idata_t idata = { UMPLG_DT_STANDARD, &d };

    // run plugin method (success)
    int res = umplg_run(m, 1000, idata.type, &idata, true);
    assert_int_equal(res, 0);

    // run plugin method (failure from umplg_run)
    res = umplg_run(m, 3000, idata.type, &idata, true);
    assert_int_equal(res, 1);

    // run plugin method (failure from the plugin itself due to args)
    res = umplg_run(m, 2000, idata.type, &idata, true);
    assert_int_equal(res, -1);

    // free buffer
    umplg_stdd_free(&d);

    // free
    umplg_free_mngr(m);
    umd_destroy(umd);
}

static void
umplg_run_test_02(void **state)
{
    umdaemon_t *umd = umd_create("test_id", "test_type");

    // create plg manager
    umplg_mngr_t *m = umplg_new_mngr();

    // load dummy plugin (success)
    umplgd_t *p = umplg_load(m, ".libs/check_umplg_plugin_01.so");
    assert_non_null(p);

    // call dummy method id = 1000

    // input/output buffer
    umplg_data_std_t d = { .items = NULL };
    umplg_stdd_init(&d);

    // column map
    umplg_data_std_items_t cmap = { .table = NULL };
    // insert columns
    umplg_data_std_item_t item_01 = { .name = "test_key_01",
                                      .value = "test_value_01" };
    umplg_data_std_item_t item_02 = { .name = "test_key_02",
                                      .value = "test_value_02" };

    // add row 01
    umplg_stdd_item_add(&cmap, &item_01);
    umplg_stdd_items_add(&d, &cmap);
    HASH_CLEAR(hh, cmap.table);

    // add row 02
    umplg_stdd_item_add(&cmap, &item_02);
    umplg_stdd_items_add(&d, &cmap);
    HASH_CLEAR(hh, cmap.table);

    // plugin input data wrapper
    umplg_idata_t idata = { UMPLG_DT_STANDARD, &d };

    // run plugin method (success)
    int res = umplg_run(m, 2000, idata.type, &idata, true);
    assert_int_equal(res, 0);

    // free buffer
    umplg_stdd_free(&d);

    // free
    umplg_free_mngr(m);
    umd_destroy(umd);
}

static void
umplg_run_test_03(void **state)
{
    umdaemon_t *umd = umd_create("test_id", "test_type");

    // create plg manager
    umplg_mngr_t *m = umplg_new_mngr();

    // load dummy plugin (success)
    umplgd_t *p = umplg_load(m, ".libs/check_umplg_plugin_01.so");
    assert_non_null(p);

    // call dummy method id = 1000

    // input/output buffer
    umplg_data_std_t d = { .items = NULL };
    umplg_stdd_init(&d);

    // column map
    umplg_data_std_items_t cmap = { .table = NULL };
    // insert columns
    umplg_data_std_item_t item_01 = { .name = "test_key_01",
                                      .value = "test_value_01" };
    umplg_data_std_item_t item_02 = { .name = "test_key_02",
                                      .value = "test_value_02" };

    // add row 01
    umplg_stdd_item_add(&cmap, &item_01);
    umplg_stdd_items_add(&d, &cmap);
    HASH_CLEAR(hh, cmap.table);

    // add row 02
    umplg_stdd_item_add(&cmap, &item_02);
    umplg_stdd_items_add(&d, &cmap);
    HASH_CLEAR(hh, cmap.table);

    // plugin input data wrapper
    umplg_idata_t idata = { UMPLG_DT_STANDARD, &d };

    // run plugin method (success)
    int res = umplg_run(m, 2000, idata.type, &idata, true);
    assert_int_equal(res, 0);

    // check result
    assert_int_equal(utarray_len(d.items), 3);
    umplg_data_std_items_t *item_res = utarray_eltptr(d.items, 2);
    assert_string_equal(item_res->table->name, "test_res_key");
    assert_string_equal(item_res->table->value, "test_res_value");

    // free buffer
    umplg_stdd_free(&d);

    // free
    umplg_free_mngr(m);
    umd_destroy(umd);
}
int
main(int argc, char **argv)
{
    const struct CMUnitTest tests[] = { cmocka_unit_test(umplg_load_test),
                                        cmocka_unit_test(umplg_run_test_01),
                                        cmocka_unit_test(umplg_run_test_02),
                                        cmocka_unit_test(umplg_run_test_03) };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
