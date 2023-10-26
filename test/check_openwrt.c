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
#include <unistd.h>
#include <math.h>
#include <cmocka_tests.h>
#include <umlua.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <libubus.h>
#include <libubox/blob.h>
#include <libubox/blobmsg.h>
#include <libubox/blobmsg_json.h>

// dummy struct for state passing
typedef struct {
    umdaemon_t *umd;
    umplg_mngr_t *m;
} test_t;

// fwd declarations
void umlua_shutdown();
int umlua_init(umplg_mngr_t *pm);
void umlua_start(umplg_mngr_t *pm);
static int run_init(void **state);
static int run_dtor(void **state);

// globals
static char plg_cfg_fname[128];
static struct ubus_context *ctx;
static struct blob_buf b;

static void
load_cfg(umplg_mngr_t *m)
{
    // load plugins configuration
    char *fname = plg_cfg_fname;
    FILE *f = fopen(fname, "r");
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
run_init(void **state)
{
    test_t *data = malloc(sizeof(test_t));
    data->umd = umd_create("test_id", "test_type");
    data->m = umplg_new_mngr();
    data->umd->perf = umc_new_ctx();
    *state = data;

    // load dummy cfg
    load_cfg(data->m);

    // init lua core
    umlua_init(data->m);

    // load MQTT plugin
    umplgd_t *p = umplg_load(data->m, ".libs/check_plg_sysagent_openwrt.so");
    assert_non_null(p);

    // start lua envs
    umlua_start(data->m);

    // connect to ubus
    ctx = ubus_connect(NULL);
    assert_non_null(ctx);

    return 0;
}

static int
run_dtor(void **state)
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
    ubus_free(ctx);
    blob_buf_free(&b);
    return 0;
}

static void
ubus_cb(struct ubus_request *req, int type, struct blob_attr *msg)
{
    // ubus output
    char *str = blobmsg_format_json(msg, true);
    assert_non_null(str);

    // add result
    char **out_str = (char **)req->priv;
    *out_str = str;
}

static void
call_list_signals(void **state)
{
    uint32_t id;

    // look for umink object
    int r = ubus_lookup_id(ctx, "umink", &id);
    assert_int_equal(r, 0);

    char *out_str = NULL;

    r = ubus_invoke(ctx,
                    id,
                    "list_signals",
                    NULL,
                    ubus_cb,
                    &out_str,
                    2000);
    assert_int_equal(r, 0);
    assert_non_null(out_str);

    // expected result
    FILE *f = fopen("./test/ubus_output_01.json", "r");
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
    char *buff = calloc(fsz, 1);
    fread(buff, fsz, 1, f);
    fclose(f);

    // get result from ubus call
    assert_string_equal(out_str, buff);

    // free
    free(out_str);
    free(buff);
}

static void
call_run_signal_w_static_output(void **state)
{
    uint32_t id;

    // look for umink object
    int r = ubus_lookup_id(ctx, "umink", &id);
    assert_int_equal(r, 0);

    blob_buf_init(&b, 0);
    blobmsg_add_json_from_string(&b, "{\"id\": \"TEST_EVENT_01\"}");
    char *out_str = NULL;

    r = ubus_invoke(ctx,
                    id,
                    "run_signal",
                    b.head,
                    ubus_cb,
                    &out_str,
                    2000);
    assert_int_equal(r, 0);
    assert_non_null(out_str);

    // get result from ubus call
    assert_string_equal(out_str, "{\"result\":\"test_data\"}");

    // free
    free(out_str);
}

static void
call_run_missing_signal(void **state)
{
    uint32_t id;

    // look for umink object
    int r = ubus_lookup_id(ctx, "umink", &id);
    assert_int_equal(r, 0);

    blob_buf_init(&b, 0);
    blobmsg_add_json_from_string(&b, "{\"id\": \"TEST_EVENT_XX\"}");
    char *out_str = NULL;

    r = ubus_invoke(ctx,
                    id,
                    "run_signal",
                    b.head,
                    ubus_cb,
                    &out_str,
                    2000);
    assert_int_equal(r, 0);
    assert_non_null(out_str);

    // get result from ubus call
    assert_string_equal(out_str, "{\"result\":\"unknown signal\"}");

    // free
    free(out_str);
}

static void
call_run_signal_w_args_return_named_arg(void **state)
{
    uint32_t id;

    // look for umink object
    int r = ubus_lookup_id(ctx, "umink", &id);
    assert_int_equal(r, 0);

    blob_buf_init(&b, 0);
    blobmsg_add_json_from_string(
        &b,
        "{\"id\": \"TEST_EVENT_03_UBUS\", "
        "\"args\": \"{\\\"test_key\\\": \\\"test_arg_data\\\"}\"}");
    char *out_str = NULL;

    r = ubus_invoke(ctx,
                    id,
                    "run_signal",
                    b.head,
                    ubus_cb,
                    &out_str,
                    2000);
    assert_int_equal(r, 0);
    assert_non_null(out_str);

    // get result from ubus call
    assert_string_equal(out_str, "{\"test_key\":\"test_arg_data\"}");

    // free
    free(out_str);
}

static void
call_run_signal_w_id_missing(void **state)
{
    uint32_t id;

    // look for umink object
    int r = ubus_lookup_id(ctx, "umink", &id);
    assert_int_equal(r, 0);

    blob_buf_init(&b, 0);
    char *out_str = NULL;

    r = ubus_invoke(ctx,
                    id,
                    "run_signal",
                    b.head,
                    ubus_cb,
                    &out_str,
                    2000);
    assert_int_equal(r, 0);
    assert_non_null(out_str);

    // get result from ubus call
    assert_string_equal(out_str, "{\"result\":\"missing arguments\"}");

    // free
    free(out_str);
}

static void
signal_match_cb(umplg_sh_t *shd, void *args)
{
    shd->min_auth_lvl = shd->min_auth_lvl ? 0 : 1;
}

static void
call_run_signal_w_sufficient_authentication_level(void **state)
{
    uint32_t id;

    // look for umink object
    int r = ubus_lookup_id(ctx, "umink", &id);
    assert_int_equal(r, 0);

    blob_buf_init(&b, 0);
    blobmsg_add_json_from_string(
        &b,
        "{\"id\": \"TEST_EVENT_01\", \"auth\": \"{\\\"flags\\\": 0}\"}");
    char *out_str = NULL;

    r = ubus_invoke(ctx,
                    id,
                    "run_signal",
                    b.head,
                    ubus_cb,
                    &out_str,
                    2000);
    assert_int_equal(r, 0);
    assert_non_null(out_str);

    // get result from ubus call
    assert_string_equal(out_str, "{\"result\":\"test_data\"}");

    // free
    free(out_str);
}

static void
call_run_signal_w_insufficient_authentication_level(void **state)
{
    // get pm
    test_t *data = *state;
    assert_non_null(data);
    umplg_mngr_t *m = data->m;

    uint32_t id;

    // set min user role level for signal to 1 (admin)
    umplg_match_signal(m, "TEST_EVENT_01", &signal_match_cb, NULL);

    // look for umink object
    int r = ubus_lookup_id(ctx, "umink", &id);
    assert_int_equal(r, 0);

    blob_buf_init(&b, 0);
    blobmsg_add_json_from_string(
        &b,
        "{\"id\": \"TEST_EVENT_01\", \"auth\": \"{\\\"flags\\\": 0}\"}");
    char *out_str = NULL;

    r = ubus_invoke(ctx,
                    id,
                    "run_signal",
                    b.head,
                    ubus_cb,
                    &out_str,
                    2000);
    assert_int_equal(r, 0);
    assert_non_null(out_str);

    // get result from ubus call
    assert_string_equal(out_str, "{\"result\":\"authentication error\"}");

    // free
    free(out_str);
}


int
main(int argc, char **argv)
{
    strcpy(plg_cfg_fname, "test/plg_cfg.json");

    const struct CMUnitTest tests[] = {
        cmocka_unit_test(call_list_signals),
        cmocka_unit_test(call_run_signal_w_static_output),
        cmocka_unit_test(call_run_missing_signal),
        cmocka_unit_test(call_run_signal_w_id_missing),
        cmocka_unit_test(call_run_signal_w_args_return_named_arg),
        cmocka_unit_test(call_run_signal_w_sufficient_authentication_level),
        cmocka_unit_test(call_run_signal_w_insufficient_authentication_level),
    };

    int r = cmocka_run_group_tests(tests, run_init, run_dtor);
    return r;

}

