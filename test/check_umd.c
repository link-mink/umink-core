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

static void
umd_test(void **state)
{
    // create daemon
    umdaemon_t *umd = umd_create("test_id", "test_type");
    assert_non_null(umd);

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

    // free
    umd_destroy(umd);
}

int
main(int argc, char **argv)
{
    const struct CMUnitTest tests[] = { cmocka_unit_test(umd_test) };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
