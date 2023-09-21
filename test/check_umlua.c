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
    umplg_free_mngr(data->m);
    umd_destroy(data->umd);
    free(data);
    return 0;
}

static void
umlua_init_test(void **state)
{
    // get pm
    test_t *data = *state;
    assert_non_null(data);
    umplg_mngr_t *m = data->m;
    m->cfg = NULL;

    // init lua core
    umlua_init(m);

    // start lua envs
    umlua_start(m);

    // stop
    umd_signal_handler(SIGTERM);

    // free
    umlua_shutdown();
}

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

// call signal handler, return static string
static void
umlua_test_01(void **state)
{
    // get pm
    test_t *data = *state;
    assert_non_null(data);
    umplg_mngr_t *m = data->m;

    // load dummy cfg
    load_cfg(m);

    // init lua core
    umlua_init(m);

    // start lua envs
    umlua_start(m);

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

    // stop
    umd_signal_handler(SIGTERM);

    // free
    json_object_put(m->cfg);
    umlua_shutdown();
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

    // load dummy cfg
    load_cfg(m);

    // init lua core
    umlua_init(m);

    // start lua envs
    umlua_start(m);

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

    // stop
    umd_signal_handler(SIGTERM);

    // free
    umplg_stdd_free(&d);
    json_object_put(m->cfg);
    umlua_shutdown();
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

    // load dummy cfg
    load_cfg(m);

    // init lua core
    umlua_init(m);

    // start lua envs
    umlua_start(m);

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

    // stop
    umd_signal_handler(SIGTERM);

    // free
    umplg_stdd_free(&d);
    json_object_put(m->cfg);
    umlua_shutdown();
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

    // load dummy cfg
    load_cfg(m);

    // init lua core
    umlua_init(m);

    // start lua envs
    umlua_start(m);

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

    // stop
    umd_signal_handler(SIGTERM);

    // free
    umplg_stdd_free(&d);
    json_object_put(m->cfg);
    umlua_shutdown();
}

int
main(int argc, char **argv)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup_teardown(umlua_init_test,
                                        umplg_run_init,
                                        umplg_run_dtor),
        cmocka_unit_test_setup_teardown(umlua_test_01,
                                        umplg_run_init,
                                        umplg_run_dtor),
        cmocka_unit_test_setup_teardown(umlua_test_02,
                                        umplg_run_init,
                                        umplg_run_dtor),
        cmocka_unit_test_setup_teardown(umlua_test_03,
                                        umplg_run_init,
                                        umplg_run_dtor),
        cmocka_unit_test_setup_teardown(umlua_test_04,
                                        umplg_run_init,
                                        umplg_run_dtor),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
