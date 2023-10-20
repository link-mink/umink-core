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
#include <stdlib.h>
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <string.h>
#include <stdio.h>
#include <cmocka_tests.h>
#include <umdaemon.h>
#include <umink_plugin.h>

// dummy struct for state passing
typedef struct {
    umdaemon_t *umd;
    umplg_mngr_t *m;
} test_t;

static int
test_sig_init_err(umplg_sh_t *shd)
{
    // error
    return 1;
}

static void
load_plugin(void **state)
{
    // get pm
    test_t *data = *state;
    umplg_mngr_t *m = data->m;

    // load dummy plugin (success)
    umplgd_t *p = umplg_load(m, ".libs/check_umplg_plugin_01.so");
    assert_non_null(p);
}

static void
load_incompatible_plugin(void **state)
{
    // get pm
    test_t *data = *state;
    umplg_mngr_t *m = data->m;

    // load dummy plugin (failure due to missing init)
    umplgd_t *p2 = umplg_load(m, ".libs/check_umplg_plugin_02.so");
    assert_null(p2);
}

static void
load_plugin_missing_file(void **state)
{

    // get pm
    test_t *data = *state;
    umplg_mngr_t *m = data->m;

    // load dummy plugin (failure due to missing file)
    umplgd_t *p3 = umplg_load(m, ".libs/check_umplg_plugin_02XX.so");
    assert_null(p3);
}

static void
load_already_loaded_plugin(void **state)
{

    // get pm
    test_t *data = *state;
    umplg_mngr_t *m = data->m;

    // reload already loaded
    umplgd_t *p4 = umplg_load(m, ".libs/check_umplg_plugin_01.so");
    assert_null(p4);
}

static void
register_signal(void **state)
{

    // get pm
    test_t *data = *state;
    umplg_mngr_t *m = data->m;

    // reg signal test
    umplg_sh_t *sh = calloc(1, sizeof(umplg_sh_t));
    sh->id = strdup("test_signal");
    sh->running = false;
    int r = umplg_reg_signal(m, sh);
    assert_int_equal(r, 0);
}

static void
register_signal_duplicate_id(void **state)
{
    // get pm
    test_t *data = *state;
    umplg_mngr_t *m = data->m;

    // reg signal test
    umplg_sh_t *sh = calloc(1, sizeof(umplg_sh_t));
    sh->id = strdup("test_signal");
    sh->running = false;
    int r = umplg_reg_signal(m, sh);
    assert_int_equal(r, 1);
    free(sh->id);
    free(sh);
}


static int
umplg_run_init(void **state)
{
    test_t *data = malloc(sizeof(test_t));
    data->umd = umd_create("test_id", "test_type");
    data->m = umplg_new_mngr();

    *state = data;
    return 0;
}

static int
umplg_run_dtor(void **state)
{
    test_t *data = *state;
    umplg_terminate_all(data->m, 0);
    umplg_free_mngr(data->m);
    umd_destroy(data->umd);
    free(data);
    return 0;
}

static void
run_plugin_command_expect_no_output(void **state)
{
    // get pm
    test_t *data = *state;
    assert_non_null(data);
    umplg_mngr_t *m = data->m;

    // call dummy method id = 1000

    // input/output buffer
    umplg_data_std_t d = { .items = NULL };
    umplg_stdd_init(&d);
    // plugin inpud data wrapper
    umplg_idata_t idata = { UMPLG_DT_STANDARD, &d };

    // run plugin method (success)
    int res = umplg_run(m, 1000, idata.type, &idata, true);
    assert_int_equal(res, 0);

    // free buffer
    umplg_stdd_free(&d);
}

static void
run_plugin_unimplemented_command(void **state)
{
    // get pm
    test_t *data = *state;
    assert_non_null(data);
    umplg_mngr_t *m = data->m;

    // call dummy method id = 1000

    // input/output buffer
    umplg_data_std_t d = { .items = NULL };
    umplg_stdd_init(&d);
    // plugin inpud data wrapper
    umplg_idata_t idata = { UMPLG_DT_STANDARD, &d };

    // run plugin method (success)
    int res = umplg_run(m, 3000, idata.type, &idata, true);
    assert_int_equal(res, 1);

    // free buffer
    umplg_stdd_free(&d);
}

static void
run_plugin_expect_err_from_plugin(void **state)
{
    // get pm
    test_t *data = *state;
    assert_non_null(data);
    umplg_mngr_t *m = data->m;

    // call dummy method id = 1000

    // input/output buffer
    umplg_data_std_t d = { .items = NULL };
    umplg_stdd_init(&d);
    // plugin inpud data wrapper
    umplg_idata_t idata = { UMPLG_DT_STANDARD, &d };

    // run plugin method (success)
    int res = umplg_run(m, 2000, idata.type, &idata, true);
    assert_int_equal(res, -1);

    // free buffer
    umplg_stdd_free(&d);
}

static void
run_plugin_w_args_expect_no_output(void **state)
{
    // get pm
    test_t *data = *state;
    assert_non_null(data);
    umplg_mngr_t *m = data->m;

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
    int r = umplg_stdd_item_add(&cmap, &item_01);
    assert_int_equal(r, 0);
    r = umplg_stdd_items_add(&d, &cmap);
    assert_int_equal(r, 0);
    // add rows NULL check
    r = umplg_stdd_items_add(NULL, &cmap);
    assert_int_equal(r, 1);
    HASH_CLEAR(hh, cmap.table);
    // add row NULL check
    r = umplg_stdd_item_add(NULL, &item_01);
    assert_int_equal(r, 1);

    // plugin input data wrapper
    umplg_idata_t idata = { UMPLG_DT_STANDARD, &d };

    // run plugin method (failure due to missing args)
    int res = umplg_run(m, 2000, idata.type, &idata, true);
    assert_int_equal(res, -1);

    // add row 02
    umplg_stdd_item_add(&cmap, &item_02);
    umplg_stdd_items_add(&d, &cmap);
    HASH_CLEAR(hh, cmap.table);

    // run plugin method (success)
    res = umplg_run(m, 2000, idata.type, &idata, true);
    assert_int_equal(res, 0);

    // free buffer
    umplg_stdd_free(NULL);
    umplg_stdd_free(&d);
}

static void
run_plugin_w_args_expect_output(void **state)
{
    // get pm
    test_t *data = *state;
    assert_non_null(data);
    umplg_mngr_t *m = data->m;

    // call dummy method id = 1000 w/args, expect res

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
}

int
main(int argc, char **argv)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(load_plugin),
        cmocka_unit_test(load_incompatible_plugin),
        cmocka_unit_test(load_plugin_missing_file),
        cmocka_unit_test(load_already_loaded_plugin),
        cmocka_unit_test(register_signal),
        cmocka_unit_test(register_signal_duplicate_id),

        cmocka_unit_test(run_plugin_command_expect_no_output),
        cmocka_unit_test(run_plugin_unimplemented_command),
        cmocka_unit_test(run_plugin_expect_err_from_plugin),
        cmocka_unit_test(run_plugin_w_args_expect_no_output),
        cmocka_unit_test(run_plugin_w_args_expect_output),
    };

    return cmocka_run_group_tests(tests, umplg_run_init, umplg_run_dtor);
}
