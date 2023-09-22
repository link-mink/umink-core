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
#include <stdio.h>
#include <signal.h>
#include <umdaemon.h>
#include <json_tokener.h>
#include <umink_plugin.h>
#include <luaconf.h>
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <cmocka_tests.h>

// fwd declarations
void umlua_shutdown();
int umlua_init(umplg_mngr_t *pm);
void umlua_start(umplg_mngr_t *pm);

// dummy struct for state passing
typedef struct {
    umdaemon_t *umd;
    umplg_mngr_t *m;
} test_t;

static void
load_cfg(umplg_mngr_t *m)
{
    // load plugins configuration
    FILE *f = fopen("test/plg_cfg.json", "r");
    assert_non_null(f);
    if (fseek(f, 0, SEEK_END) < 0) {
        fclose(f);
        fail();
    }
    int32_t fsz = ftell(f);
    if (fsz <= 0) {
        fclose(f);
        fail();
    }
    rewind(f);
    char *b = calloc(fsz + 1, 1);
    fread(b, fsz, 1, f);
    fclose(f);

    // process plugins configurataion
    m->cfg = json_tokener_parse(b);
    if (m->cfg == NULL) {
        fail_msg("ERROR: Invalid plugins configuration file");
    }
    free(b);
}

static int
umplg_run_init(void **state)
{
    test_t *data = malloc(sizeof(test_t));
    data->umd = umd_create("test_id", "test_type");
    data->m = umplg_new_mngr();

    // load dummy cfg
    load_cfg(data->m);

    // init lua core
    umlua_init(data->m);

    // start lua envs
    umlua_start(data->m);

    *state = data;
    return 0;
}

static int
umplg_run_dtor(void **state)
{
    test_t *data = *state;
    // stop
    umd_signal_handler(SIGTERM);
    // free
    umlua_shutdown();
    json_object_put(data->m->cfg);
    umplg_free_mngr(data->m);
    umd_destroy(data->umd);
    free(data);

    return 0;
}

// test signal handler (missing handler)
static void
umlua_test_signal(void **state)
{
    // get pm
    test_t *data = *state;
    assert_non_null(data);
    umplg_mngr_t *m = data->m;

    // simulate lua script (create dummy signal)

    // output buffer
    char *b = NULL;
    size_t b_sz = 0;

    // run signal
    int r = umplg_proc_signal(m, "TEST_EVENT_XX", NULL, &b, &b_sz, 0, NULL);
    assert_int_equal(r, 1);
    assert_null(b);
}

static void
signal_match_cb(umplg_sh_t *shd, void *args)
{
    shd->min_auth_lvl = shd->min_auth_lvl ? 0 : 1;
}

// test signal handler (insufficient privileges)
static void
umlua_test_user_privs(void **state)
{
    // get pm
    test_t *data = *state;
    assert_non_null(data);
    umplg_mngr_t *m = data->m;

    // simulate lua script (create dummy signal)

    // output buffer
    char *b = NULL;
    size_t b_sz = 0;

    // run signal
    int r = umplg_proc_signal(m, "TEST_EVENT_01", NULL, &b, &b_sz, 0, NULL);
    assert_int_equal(r, 0);
    assert_non_null(b);
    free(b);
    b = NULL;

    // set min user role level for signal to 1 (admin)
    umplg_match_signal(m, "TEST_EVENT_01", &signal_match_cb, NULL);

    // run signal again (expected failure)
    r = umplg_proc_signal(m, "TEST_EVENT_01", NULL, &b, &b_sz, 0, NULL);
    assert_int_equal(r, 2);
    assert_null(b);

    // revert min user role level to 0
    umplg_match_signal(m, "TEST_EVENT_01", &signal_match_cb, NULL);
}

// call signal handler, return static string
static void
umlua_test_01(void **state)
{
    // get pm
    test_t *data = *state;
    assert_non_null(data);
    umplg_mngr_t *m = data->m;

    // simulate lua script (create dummy signal)
    //
    // output buffer
    char *b = NULL;
    size_t b_sz = 0;

    // run signal
    int r = umplg_proc_signal(m, "TEST_EVENT_01", NULL, &b, &b_sz, 0, NULL);
    assert_int_equal(r, 0);
    assert_string_equal(b, "test_data");
    free(b);
}

// call signal handler with args (value without a key) and
// return arg at table index 01 (success)
static void
umlua_test_02(void **state)
{
    // get pm
    test_t *data = *state;
    assert_non_null(data);
    umplg_mngr_t *m = data->m;

    // simulate lua script (create dummy signal)

    // input data
    umplg_data_std_t d = { .items = NULL };
    umplg_stdd_init(&d);
    umplg_data_std_items_t items = { .table = NULL };
    umplg_data_std_item_t item_test = { .name = "", .value = "test_arg_data" };
    umplg_stdd_item_add(&items, &item_test);
    umplg_stdd_items_add(&d, &items);

    // output buffer
    char *b = NULL;
    size_t b_sz = 0;

    // run signal
    int r = umplg_proc_signal(m, "TEST_EVENT_02", &d, &b, &b_sz, 0, NULL);
    assert_int_equal(r, 0);
    assert_string_equal(b, "test_arg_data");
    free(b);
    HASH_CLEAR(hh, items.table);
    umplg_stdd_free(&d);
}

// call signal handler with args (value with a key) and
// return arg at table index 01 (failure)
static void
umlua_test_03(void **state)
{
    // get pm
    test_t *data = *state;
    assert_non_null(data);
    umplg_mngr_t *m = data->m;

    // simulate lua script (create dummy signal)

    // input data
    umplg_data_std_t d = { .items = NULL };
    umplg_stdd_init(&d);
    umplg_data_std_items_t items = { .table = NULL };
    umplg_data_std_item_t item_test = { .name = "test_key",
                                        .value = "test_arg_data" };
    umplg_stdd_item_add(&items, &item_test);
    umplg_stdd_items_add(&d, &items);

    // output buffer
    char *b = NULL;
    size_t b_sz = 0;

    // run signal
    int r = umplg_proc_signal(m, "TEST_EVENT_02", &d, &b, &b_sz, 0, NULL);
    assert_int_equal(r, 0);
    assert_null(b);
    HASH_CLEAR(hh, items.table);
    umplg_stdd_free(&d);
}

// call signal handler with args (value with a key) and
// return arg with a key value present (success)
static void
umlua_test_04(void **state)
{
    // get pm
    test_t *data = *state;
    assert_non_null(data);
    umplg_mngr_t *m = data->m;

    // simulate lua script (create dummy signal)

    // input data
    umplg_data_std_t d = { .items = NULL };
    umplg_stdd_init(&d);
    umplg_data_std_items_t items = { .table = NULL };
    umplg_data_std_item_t item_test = { .name = "test_key",
                                        .value = "test_arg_data" };
    umplg_stdd_item_add(&items, &item_test);
    umplg_stdd_items_add(&d, &items);

    // output buffer
    char *b = NULL;
    size_t b_sz = 0;

    // run signal
    int r = umplg_proc_signal(m, "TEST_EVENT_03", &d, &b, &b_sz, 0, NULL);
    assert_int_equal(r, 0);
    assert_string_equal(b, "test_arg_data");
    HASH_CLEAR(hh, items.table);
    free(b);
    umplg_stdd_free(&d);
}

int
main(int argc, char **argv)
{
    const struct CMUnitTest tests[] = { cmocka_unit_test(umlua_test_signal),
                                        cmocka_unit_test(umlua_test_user_privs),
                                        cmocka_unit_test(umlua_test_01),
                                        cmocka_unit_test(umlua_test_02),
                                        cmocka_unit_test(umlua_test_03),
                                        cmocka_unit_test(umlua_test_04) };

    return cmocka_run_group_tests(tests, umplg_run_init, umplg_run_dtor);
}
