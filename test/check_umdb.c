/*
 *               _____  ____ __
 *   __ ____ _  /  _/ |/ / //_/
 *  / // /  ' \_/ //    / ,<
 *  \_,_/_/_/_/___/_/|_/_/|_|
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <string.h>
#include <umdb.h>

static void
umdb_create_free_test(void **state)
{
    // no DB
    umdb_mngrd_t *m = umdb_mngr_new(NULL, false);
    assert_null(m);

    // missing DB
    m = umdb_mngr_new("./test/missing.db", false);
    assert_null(m);

    // valid DB
    m = umdb_mngr_new("./test/test.db", false);
    assert_non_null(m);
    assert_non_null(m->db);

    // free
    umdb_mngr_free(m);
}

static void
umdb_uauth_test(void **state)
{
    // load DB
    umdb_mngrd_t *m = umdb_mngr_new("./test/test.db", false);
    assert_non_null(m);

    // auth user admin with wrong pwd
    umdb_uauth_d_t res = { 0, 0, 0, NULL };
    int r = umdb_mngr_uauth(m, &res, "admin", "Apassword");
    assert_int_equal(r, 0);
    assert_int_equal(res.auth, 0);
    assert_int_equal(res.id, 1);

    // missing user
    memset(&res, 0, sizeof(umdb_uauth_d_t));
    r = umdb_mngr_uauth(m, &res, "missing_user", "Apassword");
    assert_int_equal(r, 0);
    assert_int_equal(res.id, -1);
    assert_int_equal(res.auth, -1);

    // correct username/pwd
    memset(&res, 0, sizeof(umdb_uauth_d_t));
    r = umdb_mngr_uauth(m, &res, "admin", "password");
    assert_int_equal(r, 0);
    assert_int_equal(res.id, 1);
    assert_int_equal(res.auth, 1);
}

static void
umdb_uget_test(void **state)
{
    // load DB
    umdb_mngrd_t *m = umdb_mngr_new("./test/test.db", false);
    assert_non_null(m);

    // get admin user info
    umdb_uauth_d_t res = { 0, 0, 0, NULL };
    int r = umdb_mngr_uget(m, &res, "admin");
    assert_int_equal(r, 0);
    assert_int_equal(res.id, 1);
    assert_string_equal(res.usr, "admin");

    // try missing user
    memset(&res, 0, sizeof(umdb_uauth_d_t));
    r = umdb_mngr_uget(m, &res, "admin_unknown");
    assert_int_equal(r, 0);
    assert_int_equal(res.id, -1);
    assert_int_equal(res.flags, 0);
    assert_string_equal(res.usr, "admin_unknown");
}



static void
umdb_get_set_test(void **state)
{
    // load DB (in-mem)
    umdb_mngrd_t *m = umdb_mngr_new(NULL, true);
    assert_non_null(m);

    // init store
    int r = umdb_mngr_store_init(m, "user_test_store");
    assert_int_equal(r, 0);

    // set
    r = umdb_mngr_store_set(m, "user_test_store", "test_key", "test_value");
    assert_int_equal(r, 0);

    // get (existing)
    char *res = NULL;
    size_t out_sz = 0;
    r = umdb_mngr_store_get(m, "user_test_store", "test_key", &res, &out_sz);
    assert_int_equal(r, 0);
    assert_int_equal(out_sz, 11);
    assert_string_equal("test_value", res);

    // get (missing key)
    res = NULL;
    out_sz = 0;
    r = umdb_mngr_store_get(m, "user_test_store", "test_key_missing", &res, &out_sz);
    assert_int_equal(r, 8);
    assert_int_equal(out_sz, 0);
    assert_null(res);

    // get (missing store)
    res = NULL;
    out_sz = 0;
    r = umdb_mngr_store_get(m, "user_test_store_missing", "test_key", &res, &out_sz);
    assert_int_equal(r, 3);
    assert_int_equal(out_sz, 0);
    assert_null(res);
}

int
main(int argc, char **argv)
{
    const struct CMUnitTest tests[] = { cmocka_unit_test(umdb_create_free_test),
                                        cmocka_unit_test(umdb_uauth_test),
                                        cmocka_unit_test(umdb_uget_test),
                                        cmocka_unit_test(umdb_get_set_test) };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
