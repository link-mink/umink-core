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
#include <umdb.h>

static void
create_dbm_with_nullptr_filename(void **state)
{
    // no DB
    umdb_mngrd_t *m = umdb_mngr_new(NULL, false);
    assert_null(m);
    umdb_mngr_free(m);
}

static void
create_dbm_with_unknown_file(void **state)
{
    // missing DB
    umdb_mngrd_t *m = umdb_mngr_new("./test/missing.db", false);
    assert_null(m);
    umdb_mngr_free(m);
}

static void
create_dbm(void **state)
{
    // valid DB
    umdb_mngrd_t *m = umdb_mngr_new("./test/test.db", false);
    assert_non_null(m);
    assert_non_null(m->db);
    umdb_mngr_free(m);
}

static void
free_nullptr_dbm(void **state)
{
    umdb_mngr_free(NULL);
}

static void
create_dbm_in_memory(void **state)
{
    // valid DB
    umdb_mngrd_t *m = umdb_mngr_new(NULL, true);
    assert_non_null(m);
    assert_non_null(m->db);
    umdb_mngr_free(m);
}

static void
uauth_with_wrong_pwd(void **state)
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

    // free
    umdb_mngr_free(m);
}

static void
uauth_with_nullptr_dbm(void **state)
{
    // NULL check
    umdb_uauth_d_t res = { 0, 0, 0, NULL };
    int r  = umdb_mngr_uauth(NULL, &res, "admin", "Apassword");
    assert_int_equal(r, 1);
}

static void
uauth_with_missing_user(void **state)
{
    // load DB
    umdb_mngrd_t *m = umdb_mngr_new("./test/test.db", false);
    assert_non_null(m);

    // missing user
    umdb_uauth_d_t res = { 0, 0, 0, NULL };
    memset(&res, 0, sizeof(umdb_uauth_d_t));
    int r = umdb_mngr_uauth(m, &res, "missing_user", "Apassword");
    assert_int_equal(r, 0);
    assert_int_equal(res.id, -1);
    assert_int_equal(res.auth, -1);

    // free
    umdb_mngr_free(m);
}

static void
uauth_valid_user(void **state)
{
    // load DB
    umdb_mngrd_t *m = umdb_mngr_new("./test/test.db", false);
    assert_non_null(m);

    // correct username/pwd
    umdb_uauth_d_t res = { 0, 0, 0, NULL };
    memset(&res, 0, sizeof(umdb_uauth_d_t));
    int r = umdb_mngr_uauth(m, &res, "admin", "password");
    assert_int_equal(r, 0);
    assert_int_equal(res.id, 1);
    assert_int_equal(res.auth, 1);

    // free
    umdb_mngr_free(m);
}

static void
uget_valid_user(void **state)
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

    // free
    umdb_mngr_free(m);
}

static void
uget_nullptr_dbm(void **state)
{
    // NULL check
    umdb_uauth_d_t res = { 0, 0, 0, NULL };
    int r = umdb_mngr_uget(NULL, &res, "admin");
    assert_int_equal(r, 1);
}

static void
uget_missing_user(void **state)
{
    // load DB
    umdb_mngrd_t *m = umdb_mngr_new("./test/test.db", false);
    assert_non_null(m);

    // try missing user
    umdb_uauth_d_t res = { 0, 0, 0, NULL };
    memset(&res, 0, sizeof(umdb_uauth_d_t));
    int r = umdb_mngr_uget(m, &res, "admin_unknown");
    assert_int_equal(r, 0);
    assert_int_equal(res.id, -1);
    assert_int_equal(res.flags, 0);
    assert_string_equal(res.usr, "admin_unknown");

    // free
    umdb_mngr_free(m);
}

static void
store_init(void **state)
{
    // load DB (in-mem)
    umdb_mngrd_t *m = umdb_mngr_new(NULL, true);
    assert_non_null(m);

    // init store
    int r = umdb_mngr_store_init(m, "user_test_store");
    assert_int_equal(r, 0);

    // free
    umdb_mngr_free(m);
}

static void
store_init_nullptr_dbm(void **state)
{
    // NULL check
    int r = umdb_mngr_store_init(NULL, "user_test_store");
    assert_int_equal(r, 1);
}

static void
store_set_get(void **state)
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

    // get
    char *res = NULL;
    size_t out_sz = 0;
    r = umdb_mngr_store_get(m, "user_test_store", "test_key", &res, &out_sz);
    assert_int_equal(r, 0);
    assert_int_equal(out_sz, 11);
    assert_string_equal("test_value", res);
    free(res);

    // free
    umdb_mngr_free(m);
}

static void
store_get_nullptr_dbm(void **state)
{
    char *res = NULL;
    size_t out_sz = 0;
    int r;

    r = umdb_mngr_store_get(NULL, "user_test_store", "test_key", &res, &out_sz);
    assert_int_equal(r, 1);
}

static void
store_set_nullptr_dbm(void **state)
{
    char *res = NULL;
    size_t out_sz = 0;
    int r;

    // NULL check
    r = umdb_mngr_store_set(NULL, "user_test_store", "test_key", "test_value");
    assert_int_equal(r, 1);
}


static void
store_get_missing_key(void **state)
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

    // get
    char *res = NULL;
    size_t out_sz = 0;
    r = umdb_mngr_store_get(m,
                            "user_test_store",
                            "test_key_missing",
                            &res,
                            &out_sz);
    assert_int_equal(r, 8);
    assert_int_equal(out_sz, 0);
    assert_null(res);

    // free
    umdb_mngr_free(m);
}

static void
store_get_from_missing_store(void **state)
{
    // load DB (in-mem)
    umdb_mngrd_t *m = umdb_mngr_new(NULL, true);
    assert_non_null(m);

    // init store
    int r = umdb_mngr_store_init(m, "user_test_store");
    assert_int_equal(r, 0);

    // get
    char *res = NULL;
    size_t out_sz = 0;
    r = umdb_mngr_store_get(m,
                            "user_test_store_missing",
                            "test_key",
                            &res,
                            &out_sz);
    assert_int_equal(r, 3);
    assert_int_equal(out_sz, 0);
    assert_null(res);

    // free
    umdb_mngr_free(m);
}

int
main(int argc, char **argv)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(create_dbm),
        cmocka_unit_test(create_dbm_with_unknown_file),
        cmocka_unit_test(create_dbm_with_nullptr_filename),
        cmocka_unit_test(create_dbm_in_memory),
        cmocka_unit_test(free_nullptr_dbm),
        cmocka_unit_test(uauth_valid_user),
        cmocka_unit_test(uauth_with_missing_user),
        cmocka_unit_test(uauth_with_wrong_pwd),
        cmocka_unit_test(uauth_with_nullptr_dbm),
        cmocka_unit_test(uauth_with_nullptr_dbm),
        cmocka_unit_test(uget_valid_user),
        cmocka_unit_test(uget_nullptr_dbm),
        cmocka_unit_test(uget_missing_user),
        cmocka_unit_test(uget_missing_user),
        cmocka_unit_test(store_init),
        cmocka_unit_test(store_set_get),
        cmocka_unit_test(store_get_missing_key),
        cmocka_unit_test(store_get_nullptr_dbm),
        cmocka_unit_test(store_set_nullptr_dbm),
        cmocka_unit_test(store_init_nullptr_dbm),
        cmocka_unit_test(store_get_from_missing_store)
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
