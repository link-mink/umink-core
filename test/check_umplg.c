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
#include <cmocka.h>
#include <string.h>
#include <stdio.h>
#include <umdaemon.h>
#include <umink_plugin.h>

static void
umplg_load_test(void **state)
{
    umdaemon_t *umd = umd_create("test_id", "test_type");

    // create plg manager
    umplg_mngr_t *m = umplg_new_mngr();

    // load dummy plugin (success)
    umplgd_t *p = umplg_load(m, ".libs/check_umplg_plugin_01.so");
    assert_non_null(p);

    // load dummy plugin (failure due to missing init)
    umplgd_t *p2 = umplg_load(m, ".libs/check_umplg_plugin_02.so");
    assert_null(p2);


    // free
    umplg_free_mngr(m);
    umd_destroy(umd);

}

int
main(int argc, char **argv)
{
    const struct CMUnitTest tests[] = { cmocka_unit_test(umplg_load_test) };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
