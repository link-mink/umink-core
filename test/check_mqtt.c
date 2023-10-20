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
#include <MQTTAsync.h>

// fwd declarations
void umlua_shutdown();
int umlua_init(umplg_mngr_t *pm);
void umlua_start(umplg_mngr_t *pm);
static void run_signal_expect_error(void **state);
static int mqtt_run_init(void **state);
static int mqtt_run_dtor(void **state);

// globals
static int plg_cfg_id = 0;
static bool do_pause = false;
static struct {
    char *cfg_file;
    char *cfg_error_msg;
    struct CMUnitTest tests[1];

} err_tests[] = {
    { "test/plg_cfg_mqtt_err_01.json",
      "No 'connections' array present",
      { cmocka_unit_test_setup_teardown(run_signal_expect_error,
                                        mqtt_run_init,
                                        mqtt_run_dtor) } },

    { "test/plg_cfg_mqtt_err_02.json",
      "Invalid MQTT connection type: STRING instead of JSON OBJECT",
      { cmocka_unit_test_setup_teardown(run_signal_expect_error,
                                        mqtt_run_init,
                                        mqtt_run_dtor) } },

    { "test/plg_cfg_mqtt_err_03.json",
      "MQTT connection name is missing",
      { cmocka_unit_test_setup_teardown(run_signal_expect_error,
                                        mqtt_run_init,
                                        mqtt_run_dtor) } },
    { "test/plg_cfg_mqtt_err_04.json",
      "Invalid MQTT connection name type: NUMBER instead of STRING",
      { cmocka_unit_test_setup_teardown(run_signal_expect_error,
                                        mqtt_run_init,
                                        mqtt_run_dtor) } },

    { "test/plg_cfg_mqtt_err_05.json",
      "Invalid 'proc_thread_qsize' type: STRING instead of NUMBER",
      { cmocka_unit_test_setup_teardown(run_signal_expect_error,
                                        mqtt_run_init,
                                        mqtt_run_dtor) } },

    { "test/plg_cfg_mqtt_err_06.json",
      "Invalid 'bin_upload_path' type: NUMBER instead of STRING",
      { cmocka_unit_test_setup_teardown(run_signal_expect_error,
                                        mqtt_run_init,
                                        mqtt_run_dtor) } },

    { "test/plg_cfg_mqtt_err_07.json",
      "MQTT connection 'mqtt_local' already exists",
      { cmocka_unit_test_setup_teardown(run_signal_expect_error,
                                        mqtt_run_init,
                                        mqtt_run_dtor) } },

};
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
    char *fname = plg_cfg_fname;
    if (strlen(fname) == 0) {
        fname = err_tests[plg_cfg_id].cfg_file;
        do_pause = true;

    } else {
        do_pause = false;
    }
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
mqtt_run_init(void **state)
{
    test_t *data = malloc(sizeof(test_t));
    data->umd = umd_create("test_id", "test_type");
    data->m = umplg_new_mngr();
    data->umd->perf = umc_new_ctx();

    // load dummy cfg
    load_cfg(data->m);

    // init lua core
    umlua_init(data->m);

    // load MQTT plugin
    umplgd_t *p = umplg_load(data->m, ".libs/check_plg_sysagent_mqtt.so");
    assert_non_null(p);

    // start lua envs
    umlua_start(data->m);

    *state = data;

    // pause ?
    if (do_pause) {
        sleep(1);
    }
    return 0;
}

static int
mqtt_run_dtor(void **state)
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

// dummy connect, receive MQTT PUB message
static void
test_broker_connection(void **state)
{
    sleep(2);
}

// check for MQTT PUB message
static void
test_mqtt_rx_signal_handler(void **state)
{
    // get pm
    test_t *data = *state;
    assert_non_null(data);
    umplg_mngr_t *m = data->m;

    // output buffer
    char *b = NULL;
    size_t b_sz = 0;

    // run signal
    int r = umplg_proc_signal(m, "TEST_MQTT_01", NULL, &b, &b_sz, 0, NULL);
    assert_int_equal(r, 0);
    assert_non_null(b);
    assert_string_equal(b, "mink/DEBUG_UUID/1/cmdtest_payloadmqtt_local");
    free(b);
}


static void
test_mqtt_publish_from_lua_via_generic_interface(void **state)
{
    // get pm
    test_t *data = *state;
    assert_non_null(data);
    umplg_mngr_t *m = data->m;

    // output buffer
    char *b = NULL;
    size_t b_sz = 0;

    // run signal
    int r = umplg_proc_signal(m, "TEST_MQTT_02", NULL, &b, &b_sz, 0, NULL);
    assert_int_equal(r, 0);
    assert_null(b);

    // wait for data
    sleep(1);
    // run signal (get data)
    r = umplg_proc_signal(m, "TEST_MQTT_01", NULL, &b, &b_sz, 0, NULL);
    assert_int_equal(r, 0);
    assert_non_null(b);
    assert_string_equal(b, "mink/DEBUG_UUID/1/cmdtest_dummy_datamqtt_local");
    free(b);
}

static void
test_mqtt_publish_from_lua_via_submodule_methods(void **state)
{
    // get pm
    test_t *data = *state;
    assert_non_null(data);
    umplg_mngr_t *m = data->m;

    // output buffer
    char *b = NULL;
    size_t b_sz = 0;

    // run signal
    int r = umplg_proc_signal(m, "TEST_MQTT_03", NULL, &b, &b_sz, 0, NULL);
    assert_int_equal(r, 0);
    assert_non_null(b);
    assert_string_equal(b, "OK");
    free(b);
    b = NULL;

    // wait for data
    sleep(1);
    // run signal (get data)
    r = umplg_proc_signal(m, "TEST_MQTT_01", NULL, &b, &b_sz, 0, NULL);
    assert_int_equal(r, 0);
    assert_non_null(b);
    assert_string_equal(b, "mink/DEBUG_UUID/1/cmdtest_dummy_data_02mqtt_local");
    free(b);
}

static void
test_mqtt_binary_file_upload(void **state)
{
    // get pm
    test_t *data = *state;
    assert_non_null(data);
    umplg_mngr_t *m = data->m;

    // output buffer
    char *b = NULL;
    size_t b_sz = 0;

    // run signal
    int r = umplg_proc_signal(m, "TEST_MQTT_04", NULL, &b, &b_sz, 0, NULL);
    assert_int_equal(r, 0);
    assert_non_null(b);
    // file uuid returned
    if (strlen(b) != 73) {
        fail();
    }

    // tokenize
    char *valid_uuid = NULL;
    char *incomplete_uuid = NULL;
    char *token = NULL;
    char *saveptr = NULL;

    // get valid uuid (all chunks)
    token = strtok_r(b, ":", &saveptr);
    assert_non_null(token);
    valid_uuid = strdup(token);

    // get incomplete uuid (only first chunk)
    token = strtok_r(NULL, ":", &saveptr);
    assert_non_null(token);
    incomplete_uuid = strdup(token);

    // wait for data
    sleep(7);

    // check is file was uploaded
    char fp[strlen("/tmp/mink.bin") + 36 + 2];
    fp[0] = '\0';
    strcat(fp, "/tmp/mink.bin");
    strcat(fp, "/");
    strcat(fp, b);
    FILE *f = fopen(fp, "r");
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
    fclose(f);
    assert_int_equal(fsz, 1024);

    // incomplete file should have been removed
    sleep(5);
    strcpy(&fp[strlen(fp) - 36], incomplete_uuid);
    FILE *f2 = fopen(fp, "r");
    assert_null(f2);

    // cleanup
    free(b);
    free(valid_uuid);
    free(incomplete_uuid);
}

static void
run_signal_expect_error(void **state)
{
    // get pm
    test_t *data = *state;
    assert_non_null(data);
    umplg_mngr_t *m = data->m;

    // output buffer
    char *b = NULL;
    size_t b_sz = 0;

    // run signal
    int r = umplg_proc_signal(m, "TEST_MQTT_03", NULL, &b, &b_sz, 0, NULL);
    assert_int_equal(r, 0);
    assert_non_null(b);
    char *cmp_str = "ERR";
    if (plg_cfg_id == 6) {
        cmp_str = "OK";
    }
    assert_string_equal(b, cmp_str);
    free(b);
    plg_cfg_id++;
}

int
main(int argc, char **argv)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_broker_connection),
        cmocka_unit_test(test_mqtt_rx_signal_handler),
        cmocka_unit_test(test_mqtt_publish_from_lua_via_generic_interface),
        cmocka_unit_test(test_mqtt_publish_from_lua_via_submodule_methods),
        cmocka_unit_test(test_mqtt_binary_file_upload)
    };

    // group 01
    strcpy(plg_cfg_fname, "test/plg_cfg_mqtt.json");
    int r = cmocka_run_group_tests(tests, mqtt_run_init, mqtt_run_dtor);
    if (0 == 0) {

        for (int i = 0; i < 7; i++) {
            plg_cfg_fname[0] = '\0';
            r +=
                cmocka_run_group_tests_name(err_tests[i].cfg_error_msg,
                                            err_tests[i].tests,
                                            NULL,
                                            NULL);
        }
    }
    return r;

}

