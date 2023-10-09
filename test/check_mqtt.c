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

// globals
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
mqtt_test_connect(void **state)
{
    sleep(2);
}

// check for MQTT PUB message
static void
mqtt_test_01(void **state)
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

int
main(int argc, char **argv)
{
    const struct CMUnitTest tests[] = { cmocka_unit_test(mqtt_test_connect),
                                        cmocka_unit_test(mqtt_test_01) };

    strcpy(plg_cfg_fname, "test/plg_cfg_mqtt.json");
    return cmocka_run_group_tests(tests, mqtt_run_init, mqtt_run_dtor);
}

