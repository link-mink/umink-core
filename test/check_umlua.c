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
#include <umlua.h>
#include <sys/socket.h>
#include <sys/un.h>

// fwd declarations
void umlua_shutdown();
int umlua_init(umplg_mngr_t *pm);
void umlua_start(umplg_mngr_t *pm);

// test lua env manager
static struct lua_env_mngr *tmp_lenvm;
struct lua_env_mngr *lenvm_new();
void lenvm_free(struct lua_env_mngr *m);
struct lua_env_d *lenvm_get_envd(struct lua_env_mngr *lem, const char *n);
struct lua_env_d *lenvm_new_envd(struct lua_env_mngr *lem,
                                 struct lua_env_d *env);
bool lenvm_envd_exists(struct lua_env_mngr *lem, const char *n);
void lenvm_process_envs(struct lua_env_mngr *lem,
                        void (*f)(struct lua_env_d *env));
struct lua_env_d *
lenvm_del_envd(struct lua_env_mngr *lem, const char *n, bool th_safe);
static char plg_cfg_fname[128];

// dummy struct for state passing
typedef struct {
    umdaemon_t *umd;
    umplg_mngr_t *m;
} test_t;

static void
load_cfg(umplg_mngr_t *m)
{
    // load plugins configuration
    FILE *f = fopen(plg_cfg_fname, "r");
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
    // load dummy plugin (implements CMD_LUA_CALL)
    umplgd_t *p = umplg_load(data->m, ".libs/check_umplg_plugin_01.so");
    assert_non_null(p);

    // start lua envs
    umlua_start(data->m);

    *state = data;
    return 0;
}

static void
shutdown_envs(struct lua_env_d *env)
{
    printf("Freeing env [%s]\n", env->name);
    // cleanup
    lenvm_del_envd(tmp_lenvm, env->name, false);
    free(env->name);
    free(env->path);
    free(env->sgnl_perf);
    free(env);
}

static int
umplg_run_dtor(void **state)
{
    test_t *data = *state;

    // tmp lenvm
    if (tmp_lenvm != NULL) {
        lenvm_process_envs(tmp_lenvm, &shutdown_envs);
        lenvm_free(tmp_lenvm);
        tmp_lenvm = NULL;
    }

    // stop
    umd_signal_handler(SIGTERM);
    umplg_terminate_all(data->m, 0);
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
run_missing_signal(void **state)
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
    // NULL check
    r = umplg_proc_signal(m, NULL, NULL, &b, &b_sz, 0, NULL);
    assert_int_equal(r, UMPLG_RES_UNKNOWN_SIGNAL);
    assert_null(b);
}

static void
create_lua_environment_manager(void **state)
{
    // new env mngr
    tmp_lenvm = lenvm_new();
    assert_non_null(tmp_lenvm);
}

static void
free_lua_environment_manager(void **state)
{
    lenvm_process_envs(tmp_lenvm, &shutdown_envs);
    lenvm_free(tmp_lenvm);
}

static void
create_lua_environment(void **state)
{
    tmp_lenvm = lenvm_new();
    assert_non_null(tmp_lenvm);

    // new dummy env
    struct lua_env_d *env = calloc(1, sizeof(struct lua_env_d));
    env->name = strdup("test_env_id");
    struct lua_env_d *env_tmp = lenvm_new_envd(tmp_lenvm, env);
    assert_ptr_equal(env, env_tmp);
}

static void
create_lua_environment_duplicate_id(void **state)
{
    // get old env
    struct lua_env_d *env_01 = lenvm_get_envd(tmp_lenvm, "test_env_id");
    assert_non_null(env_01);

    // new dummy env
    struct lua_env_d *env_02 = calloc(1, sizeof(struct lua_env_d));
    env_02->name = strdup("test_env_id");
    struct lua_env_d *env_tmp = lenvm_new_envd(tmp_lenvm, env_02);
    assert_ptr_equal(env_01, env_tmp);
    free(env_02->name);
    free(env_02);
}

static void
look_for_lua_environment(void **state)
{
    bool found = lenvm_envd_exists(tmp_lenvm, "test_env_id");
    assert_int_equal(found, 1);
}

static void
look_for_missing_lua_environment(void **state)
{
    bool found = lenvm_envd_exists(tmp_lenvm, "test_env_id_XX");
    assert_int_equal(found, 0);
}

static void
get_lua_environment(void **state)
{
    struct lua_env_d *env = lenvm_get_envd(tmp_lenvm, "test_env_id");
    assert_non_null(env);
}

static void
get_missing_lua_environment(void **state)
{
    struct lua_env_d *env = lenvm_get_envd(tmp_lenvm, "test_env_id_XX");
    assert_null(env);
}

static void
delete_lua_environment(void **state)
{
    struct lua_env_d *env = lenvm_del_envd(tmp_lenvm, "test_env_id", false);
    assert_non_null(env);
    free(env->name);
    free(env);
}

static void
delete_missing_lua_environment(void **state)
{
    struct lua_env_d *env = lenvm_del_envd(tmp_lenvm, "test_env_id_XX", false);
    assert_null(env);
}
static void
signal_match_cb(umplg_sh_t *shd, void *args)
{
    shd->min_auth_lvl = shd->min_auth_lvl ? 0 : 1;
}

static void
run_signal_w_sufficient_authentication_level(void **state)
{
    // get pm
    test_t *data = *state;
    assert_non_null(data);
    umplg_mngr_t *m = data->m;

    // output buffer
    char *b = NULL;
    size_t b_sz = 0;

    // run signal
    int r = umplg_proc_signal(m, "TEST_EVENT_01", NULL, &b, &b_sz, 0, NULL);
    assert_int_equal(r, 0);
    assert_non_null(b);
    free(b);
    b = NULL;
}

static void
run_signal_w_insufficient_authentication_level(void **state)
{
    // get pm
    test_t *data = *state;
    assert_non_null(data);
    umplg_mngr_t *m = data->m;

    // output buffer
    char *b = NULL;
    size_t b_sz = 0;

    // set min user role level for signal to 1 (admin)
    umplg_match_signal(m, "TEST_EVENT_01", &signal_match_cb, NULL);

    // run signal again (expected failure)
    int r = umplg_proc_signal(m, "TEST_EVENT_01", NULL, &b, &b_sz, 0, NULL);
    assert_int_equal(r, UMPLG_RES_AUTH_ERROR);
    assert_null(b);

    // revert min user role level to 0
    umplg_match_signal(m, "TEST_EVENT_01", &signal_match_cb, NULL);

}

// call signal handler, return static string
static void
run_signal_w_static_output(void **state)
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
static void
run_signal_w_multiple_names(void **state)
{
    // get pm
    test_t *data = *state;
    assert_non_null(data);
    umplg_mngr_t *m = data->m;

    // output buffer
    char *b = NULL;
    size_t b_sz = 0;
    // run signal
    int r = umplg_proc_signal(m, "TEST_EVENT_01", NULL, &b, &b_sz, 0, NULL);
    assert_int_equal(r, 0);
    assert_string_equal(b, "test_data");
    free(b);

    // call again, check signal handler with multiple names
    r = umplg_proc_signal(m, "TEST_EVENT_01_02", NULL, &b, &b_sz, 0, NULL);
    assert_int_equal(r, 0);
    assert_string_equal(b, "test_data");
    free(b);
}

// call signal handler with args (value with a key) and
// return arg at table index 01 (failure)
static void
run_signal_w_args_return_missing_arg(void **state)
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
run_signal_w_args_return_named_arg(void **state)
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
run_lua_env_thread(void **state)
{
    // get pm
    test_t *data = *state;

    // sleep for 2 seconds (counters value should be at 4 || 5)
    sleep(2);

    // get custom counter
    umc_t *c = umc_get(NULL, "test_env_counter", true);
    assert_null(c);
    c = umc_get(data->umd->perf, "test_env_counter", true);
    assert_non_null(c);
    if (c->values.last.value < 4 || c->values.last.value > 5) {
        fail_msg("test counter should be >= 4 (freq/duration = 500msec/2s)");
    }
    int expc = c->values.last.value;

    // check runtime counters (execution count)
    c = umc_get(data->umd->perf, "lua.environment.TEST_ENV.count", true);
    assert_non_null(c);
    assert_int_equal(c->values.last.value, expc);

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

//  check failed env
static void
check_failed_lua_environment_thread(void **state)
{
    sleep(2);
    // get custom counter
    umc_t *c = umc_get(NULL, "test_env_counter", true);
    assert_null(c);
}

//  check failed signal
static void
run_signal_w_malformed_lua_script_nil_value(void **state)
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
    umplg_data_std_item_t item_test = { .name = "", .value = "TEST_EVENT_01" };
    umplg_stdd_item_add(&items, &item_test);
    umplg_stdd_items_add(&d, &items);

    // run signal
    int r = umplg_proc_signal(m, "TEST_EVENT_02", &d, &b, &b_sz, 0, NULL);
    assert_int_equal(r, 0);
    assert_non_null(b);
    assert_string_equal(
        b,
        "0.0:test/test_event_01_err.lua:1: attempt to call a nil "
        "value (global 'rsdfeturn')");
    free(b);
    HASH_CLEAR(hh, items.table);
    umplg_stdd_free(&d);
}

//  check failed signal
static void
run_signal_w_malformed_lua_script_syntax(void **state)
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
                                        .value = "TEST_EVENT_01_SYNTAX_ERR" };
    umplg_stdd_item_add(&items, &item_test);
    umplg_stdd_items_add(&d, &items);

    // run signal
    int r = umplg_proc_signal(m, "TEST_EVENT_02", &d, &b, &b_sz, 0, NULL);
    assert_int_equal(r, 0);
    assert_non_null(b);
    assert_string_equal(b, "4.0:");
    free(b);
    HASH_CLEAR(hh, items.table);
    umplg_stdd_free(&d);
}

//  check counters again (check_umc), this time from lua script
static void
run_signal_check_umc_from_lua(void **state)
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
run_signal_call_signal_from_another_signal(void **state)
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

//  check signal call from another signal (admin)
static void
run_signal_call_admin_signal_from_another_signal(void **state)
{
    // get pm
    test_t *data = *state;
    umplg_mngr_t *m = data->m;

    // output buffer
    char *b = NULL;
    size_t b_sz = 0;
    int r;

    // run signal
    r = umplg_proc_signal(m, "TEST_EVENT_05_ADMIN", NULL, &b, &b_sz, 0, NULL);
    assert_int_equal(r, 0);
    assert_non_null(b);
    assert_string_equal(b, "test_data");
    free(b);
}

//  check signal recursion check
static void
run_signal_prevent_signal_recursion(void **state)
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
run_signal_check_arg_levels_w_thread_local_lref(void **state)
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
    umplg_data_std_item_t item_test = { .name = "", .value = "test_arg_data" };
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
run_signal_check_umdb_from_lua(void **state)
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
    umplg_data_std_item_t item_test = { .name = "", .value = "set" };
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
run_signal_check_cmd_call_w_generic_interface(void **state)
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
    umplg_data_std_item_t item_test = { .name = "", .value = "set" };
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

//  check cmd_call (from lua script this time)
static void
run_signal_check_lua_submodule_from_umink_plugin(void **state)
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
    umplg_data_std_item_t item_test = { .name = "", .value = "set" };
    umplg_stdd_item_add(&items, &item_test);
    umplg_stdd_items_add(&d, &items);

    // run signal (set)
    int r = umplg_proc_signal(m, "TEST_EVENT_10", &d, &b, &b_sz, 0, NULL);
    assert_int_equal(r, 0);
    assert_non_null(b);
    assert_string_equal(b, "test_method_return_string_01nilarg_test");
    HASH_CLEAR(hh, items.table);

    free(b);
    umplg_stdd_free(&d);
}

//  check domain socket cli
static void
run_signal_via_unix_domain_socket(void **state)
{
    const char *socket_path = "/tmp/umink.sock";
    int sock = 0;
    int data_len = 0;
    struct sockaddr_un remote;
    char recv_msg[128];
    char send_msg[128];

    memset(recv_msg, 0, sizeof(recv_msg));
    memset(send_msg, 0, sizeof(send_msg));

    // create socket
    if ((sock = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        fail();
    }

    // connect
    remote.sun_family = AF_UNIX;
    strcpy(remote.sun_path, socket_path);
    data_len = strlen(remote.sun_path) + sizeof(remote.sun_family);

    if (connect(sock, (struct sockaddr *)&remote, data_len) == -1) {
        fail();
    }

    // send msg
    strcpy(send_msg, "return M.signal(\"TEST_EVENT_01\")");

    if (send(sock, send_msg, strlen(send_msg) * sizeof(char), 0) == -1) {
        printf("Client: Error on send() call \n");
        fail();
    }

    // receive
    data_len = recv(sock, recv_msg, sizeof(recv_msg), 0);
    assert_int_equal(data_len, 10);
    assert_string_equal(recv_msg, "test_data");
    close(sock);
}

int
main(int argc, char **argv)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(run_lua_env_thread),
        cmocka_unit_test(run_missing_signal),
        cmocka_unit_test(create_lua_environment_manager),
        cmocka_unit_test(free_lua_environment_manager),
        cmocka_unit_test(create_lua_environment),
        cmocka_unit_test(create_lua_environment_duplicate_id),
        cmocka_unit_test(look_for_lua_environment),
        cmocka_unit_test(look_for_missing_lua_environment),
        cmocka_unit_test(get_lua_environment),
        cmocka_unit_test(get_missing_lua_environment),
        cmocka_unit_test(delete_lua_environment),
        cmocka_unit_test(delete_missing_lua_environment),
        cmocka_unit_test(run_signal_w_sufficient_authentication_level),
        cmocka_unit_test(run_signal_w_insufficient_authentication_level),
        cmocka_unit_test(run_signal_w_static_output),
        cmocka_unit_test(run_signal_w_multiple_names),
        cmocka_unit_test(run_signal_w_args_return_missing_arg),
        cmocka_unit_test(run_signal_w_args_return_named_arg),
        cmocka_unit_test(run_signal_check_umc_from_lua),
        cmocka_unit_test(run_signal_call_signal_from_another_signal),
        cmocka_unit_test(run_signal_call_admin_signal_from_another_signal),
        cmocka_unit_test(run_signal_prevent_signal_recursion),
        cmocka_unit_test(run_signal_check_arg_levels_w_thread_local_lref),
        cmocka_unit_test(run_signal_check_umdb_from_lua),
        cmocka_unit_test(run_signal_check_cmd_call_w_generic_interface),
        cmocka_unit_test(run_signal_check_lua_submodule_from_umink_plugin),
        cmocka_unit_test(run_signal_via_unix_domain_socket)
    };

    const struct CMUnitTest tests_02[] = {
        cmocka_unit_test(check_failed_lua_environment_thread),
        cmocka_unit_test(run_signal_w_malformed_lua_script_nil_value),
        cmocka_unit_test(run_signal_w_malformed_lua_script_syntax),

    };

    // *** valid scripts ***
    // conserve memory
    strcpy(plg_cfg_fname, "test/plg_cfg.json");
    int r = cmocka_run_group_tests_name(
        "Running VALID configuration with conserve_mem ENABLED",
        tests,
        umplg_run_init,
        umplg_run_dtor);
    if (r == 0) {
        // do not conserve memory
        strcpy(plg_cfg_fname, "test/plg_cfg_ncs.json");
        r += cmocka_run_group_tests_name(
            "Running VALID configuration with conserve_mem DISABLED",
            tests,
            umplg_run_init,
            umplg_run_dtor);
    }
    // *** scripts with errors ***
    // conserve memory
    if (r == 0) {
        strcpy(plg_cfg_fname, "test/plg_cfg_err.json");
        r += cmocka_run_group_tests_name(
            "Running INVALID configuration with conserve_mem ENABLED",
            tests_02,
            umplg_run_init,
            umplg_run_dtor);

        // do not conserve memory
        if (r == 0) {
            strcpy(plg_cfg_fname, "test/plg_cfg_err_ncs.json");
            r += cmocka_run_group_tests_name(
                "Running INVALID configuration with conserve_mem DISABLED",
                tests_02,
                umplg_run_init,
                umplg_run_dtor);
        }
    }

    return r;
}
