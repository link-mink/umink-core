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
#include <stdio.h>
#include <umdaemon.h>
#include <signal.h>
#include <unistd.h>

// dummy struct for state passing
typedef struct {
    umdaemon_t *umd;
} test_t;

static int
umplg_run_init(void **state)
{
    test_t *data = malloc(sizeof(test_t));
    data->umd = umd_create("test_id", "test_type");
    *state = data;
    return 0;
}

static int
umplg_run_dtor(void **state)
{
    test_t *data = *state;
    // free
    umd_destroy(data->umd);
    free(data);
    return 0;
}

static void
umd_test_01(void **state)
{
    // get data
    test_t *data = *state;
    umdaemon_t *umd = data->umd;

    // check id and type
    assert_string_equal(umd->id, "test_id");
    assert_string_equal(umd->type, "test_type");
    assert_string_equal(umd->fqn, "mink.test_type.test_id");

    // chage id (success)
    umd_set_id(umd, "new_id");
    assert_string_equal(umd->id, "new_id");

    // chage id (failure)
    umd_set_id(umd, "new_id000000000000000000000000");
    assert_string_equal(umd->id, "new_id");
}

static void
umd_test_02(void **state)
{
    // get data
    test_t *data = *state;
    umdaemon_t *umd = data->umd;

    // set log level
    umd_set_log_level(umd, UMD_LLT_DEBUG);
    assert_int_equal(umd->log_level, UMD_LLT_DEBUG);
}

static void *
kill_thread(void *args)
{
    sleep(1);
    UM_ATOMIC_COMP_SWAP(&UMD->is_terminated, 0, 1);
    return NULL;
}

static void
umd_test_03(void **state)
{
    // get data
    test_t *data = *state;
    umdaemon_t *umd = data->umd;

    // new thread
    pthread_t th;
    pthread_create(&th, NULL, &kill_thread, NULL);

    // start test
    umd_start(umd);
    umd_loop(umd);
}

int
main(int argc, char **argv)
{
    const struct CMUnitTest tests[] = { cmocka_unit_test(umd_test_01),
                                        cmocka_unit_test(umd_test_02),
                                        cmocka_unit_test(umd_test_03) };

    return cmocka_run_group_tests(tests, umplg_run_init, umplg_run_dtor);
}
