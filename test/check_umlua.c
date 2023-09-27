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
#include <unistd.h>
#include <math.h>
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
    data->umd->perf = umc_new_ctx();

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
    umc_free_ctx(data->umd->perf);
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
    b = NULL;

    // call again, check signal handler with multiple names
    r = umplg_proc_signal(m, "TEST_EVENT_01_02", NULL, &b, &b_sz, 0, NULL);
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

//  check lua env running every 500msec, inc counter by 1, expect 4
static void
umlua_test_env_01(void **state)
{
    // get pm
    test_t *data = *state;

    // sleep for 2 seconds (counters value should be at 4)
    sleep(2);

    // get custom counter
    umc_t *c = umc_get(data->umd->perf, "test_env_counter", true);
    assert_non_null(c);
    assert_int_equal(c->values.last.value, 4);

    // check runtime counters (execution count)
    c = umc_get(data->umd->perf, "lua.environment.TEST_ENV.count", true);
    assert_non_null(c);
    assert_int_equal(c->values.last.value, 4);

    // check runtime counters (rate, should be around 2 per second for
    // a 500msec frequency)
    double cr = umc_get_rate(c, true);
    assert_int_equal(ceil(cr), 2);

    // check runtime counters (error count)
    c = umc_get(data->umd->perf, "lua.environment.TEST_ENV.error", true);
    assert_non_null(c);
    assert_int_equal(c->values.last.value, 0);

    // check runtime counters (lag)
    c = umc_get(data->umd->perf, "lua.environment.TEST_ENV.lag", true);
    assert_non_null(c);
    if (c->values.last.value == 0) {
        fail();
    }
}

//  check counters again (check_umc), this time from lua script
static void
umlua_test_05(void **state)
{
    // get pm
    test_t *data = *state;
    umplg_mngr_t *m = data->m;

    // output buffer
    char *b = NULL;
    size_t b_sz = 0;

    // run signal
    int r = umplg_proc_signal(m, "TEST_EVENT_04", NULL, &b, &b_sz, 0, NULL);
    assert_int_equal(r, 0);
    assert_non_null(b);

    // get INC counter "test_05_c1"
    umc_t *c = umc_get(data->umd->perf, "test_05_c1", true);
    assert_non_null(c);
    assert_int_equal(c->values.last.value, 2);

    // get GAUGE counter "test_05_c2"
    c = umc_get(data->umd->perf, "test_05_c2", true);
    assert_non_null(c);
    assert_int_equal(c->values.last.value, 998);
    assert_int_equal(c->values.max, 999);

    // cehck result (perf_match test)
    assert_string_equal(b, "test_05_c1test_05_c2");
    free(b);
}

//  check signal call from another signal
static void
umlua_test_06(void **state)
{
    // get pm
    test_t *data = *state;
    umplg_mngr_t *m = data->m;

    // output buffer
    char *b = NULL;
    size_t b_sz = 0;

    // run signal
    int r = umplg_proc_signal(m, "TEST_EVENT_05", NULL, &b, &b_sz, 0, NULL);
    assert_int_equal(r, 0);
    assert_non_null(b);
    assert_string_equal(b, "test_data");
    free(b);
}

//  check signal recursion check
static void
umlua_test_07(void **state)
{
    // get pm
    test_t *data = *state;
    umplg_mngr_t *m = data->m;

    // output buffer
    char *b = NULL;
    size_t b_sz = 0;

    // run signal
    int r = umplg_proc_signal(m, "TEST_EVENT_06", NULL, &b, &b_sz, 0, NULL);
    assert_int_equal(r, 0);
    assert_non_null(b);
    assert_string_equal(b, "ERR [TEST_EVENT_06]: signal recursion prevented");
    free(b);
}

//  check multiple mink_stdd arg levels (Lref)
static void
umlua_test_08(void **state)
{
    // get pm
    test_t *data = *state;
    umplg_mngr_t *m = data->m;

    // output buffer
    char *b = NULL;
    size_t b_sz = 0;

    // input data
    umplg_data_std_t d = { .items = NULL };
    umplg_stdd_init(&d);
    umplg_data_std_items_t items = { .table = NULL };
    umplg_data_std_item_t item_test = { .name = "",
                                        .value = "test_arg_data" };
    umplg_stdd_item_add(&items, &item_test);
    umplg_stdd_items_add(&d, &items);

    // run signal
    int r = umplg_proc_signal(m, "TEST_EVENT_07", &d, &b, &b_sz, 0, NULL);
    assert_int_equal(r, 0);
    assert_non_null(b);
    assert_string_equal(b, "test_arg_dataevent_02_args");
    free(b);
    HASH_CLEAR(hh, items.table);
    umplg_stdd_free(&d);
}

//  check db set/get methods (from lua script this time)
static void
umlua_test_09(void **state)
{
    // get pm
    test_t *data = *state;
    umplg_mngr_t *m = data->m;

    // output buffer
    char *b = NULL;
    size_t b_sz = 0;

    // input data
    umplg_data_std_t d = { .items = NULL };
    umplg_stdd_init(&d);
    umplg_data_std_items_t items = { .table = NULL };
    umplg_data_std_item_t item_test = { .name = "",
                                        .value = "set" };
    umplg_stdd_item_add(&items, &item_test);
    umplg_stdd_items_add(&d, &items);

    // run signal (set)
    int r = umplg_proc_signal(m, "TEST_EVENT_08", &d, &b, &b_sz, 0, NULL);
    assert_int_equal(r, 0);
    assert_int_equal(b_sz, 0);
    assert_null(b);
    HASH_CLEAR(hh, items.table);

    // run signal (get)
    umplg_stdd_free(&d);
    d.items = NULL;

    umplg_stdd_init(&d);
    item_test.name = "";
    item_test.value = "get";
    umplg_stdd_item_add(&items, &item_test);
    umplg_stdd_items_add(&d, &items);

    r = umplg_proc_signal(m, "TEST_EVENT_08", &d, &b, &b_sz, 0, NULL);
    assert_int_equal(r, 0);
    assert_non_null(b);
    assert_string_equal(b, "test_value");
    free(b);
    b = NULL;
    HASH_CLEAR(hh, items.table);
    umplg_stdd_free(&d);
}

//  check cmd_call (from lua script this time)
static void
umlua_test_10(void **state)
{
    // get pm
    test_t *data = *state;
    umplg_mngr_t *m = data->m;

    // load dummy plugin (implements CMD_LUA_CALL)
    umplgd_t *p = umplg_load(m, ".libs/check_umplg_plugin_01.so");
    assert_non_null(p);

    // output buffer
    char *b = NULL;
    size_t b_sz = 0;

    // input data
    umplg_data_std_t d = { .items = NULL };
    umplg_stdd_init(&d);
    umplg_data_std_items_t items = { .table = NULL };
    umplg_data_std_item_t item_test = { .name = "",
                                        .value = "set" };
    umplg_stdd_item_add(&items, &item_test);
    umplg_stdd_items_add(&d, &items);

    // run signal (set)
    int r = umplg_proc_signal(m, "TEST_EVENT_09", &d, &b, &b_sz, 0, NULL);
    assert_int_equal(r, 0);
    assert_non_null(b);
    assert_string_equal(b, "test_res_value");
    HASH_CLEAR(hh, items.table);

    free(b);
    umplg_stdd_free(&d);
}

int
main(int argc, char **argv)
{
    const struct CMUnitTest tests[] = { cmocka_unit_test(umlua_test_env_01),
                                        cmocka_unit_test(umlua_test_signal),
                                        cmocka_unit_test(umlua_test_user_privs),
                                        cmocka_unit_test(umlua_test_01),
                                        cmocka_unit_test(umlua_test_02),
                                        cmocka_unit_test(umlua_test_03),
                                        cmocka_unit_test(umlua_test_04),
                                        cmocka_unit_test(umlua_test_05),
                                        cmocka_unit_test(umlua_test_06),
                                        cmocka_unit_test(umlua_test_07),
                                        cmocka_unit_test(umlua_test_08),
                                        cmocka_unit_test(umlua_test_09),
                                        cmocka_unit_test(umlua_test_10) };

    return cmocka_run_group_tests(tests, umplg_run_init, umplg_run_dtor);
}
