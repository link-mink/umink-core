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
#include <cmocka.h>
#include <cmocka_tests.h>
#include <stdio.h>
#include <umcounters.h>

static void
umc_create_free_test(void **state)
{
    // create ctx
    umc_ctx_t *umc = umc_new_ctx();
    assert_non_null(umc);

    // free
    umc_free_ctx(NULL);
    umc_free_ctx(umc);
}

static void
umc_create_cntr_test(void **state)
{
    // create ctx
    umc_ctx_t *umc = umc_new_ctx();
    assert_non_null(umc);

    // NULL check
    umc_t *c = umc_new_counter(NULL, "test_inc_counter", UMCT_INCREMENTAL);
    assert_null(c);

    // new incremental counter
    c = umc_new_counter(umc, "test_inc_counter", UMCT_INCREMENTAL);
    assert_non_null(c);
    assert_string_equal(c->idp, "test_inc_counter");
    assert_int_equal(c->type, UMCT_INCREMENTAL);
    // NULL check
    c = umc_new_counter(NULL, "test_inc_counter", UMCT_INCREMENTAL);
    assert_null(c);

    // new gauge counter
    umc_t *c2 = umc_new_counter(umc, "test_gauge_counter", UMCT_GAUGE);
    assert_non_null(c2);
    assert_string_equal(c2->idp, "test_gauge_counter");
    assert_int_equal(c2->type, UMCT_GAUGE);

    // free
    umc_free_ctx(umc);
}

static void
umc_inc_cntr_test(void **state)
{
    // create ctx
    umc_ctx_t *umc = umc_new_ctx();
    assert_non_null(umc);

    // new incremental counter
    umc_t *c = umc_new_counter(umc, "test_inc_counter", UMCT_INCREMENTAL);
    assert_non_null(c);
    assert_string_equal(c->idp, "test_inc_counter");
    assert_int_equal(c->type, UMCT_INCREMENTAL);

    // last value
    assert_int_equal(c->values.last.value, 0);
    assert_int_equal(c->values.max, 0);

    // inc
    umc_inc(c, 10);
    assert_int_equal(c->values.last.value, 10);
    assert_int_equal(c->values.max, 10);
    // NULL check
    umc_inc(NULL, 1);
    assert_int_equal(c->values.max, 10);

    // check get/set/inc for INCREMENTAL
    // expected failure
    umc_t *c2 = umc_get_inc(umc, "test_inc_counter_unknown", 10, false);
    assert_null(c2);
    // expected success
    c2 = umc_get_inc(umc, "test_inc_counter", 5, false);
    assert_non_null(c2);
    // w/locks
    c2 = umc_get_inc(umc, "test_inc_counter", 5, true);
    assert_non_null(c2);
    assert_int_equal(c->values.last.value, 20);

    // check umc_set
    // NULL check
    c2 = umc_get_set(NULL, "test_inc_counter", 30, false);
    assert_null(c2);
    // expected failure
    c2 = umc_get_set(umc, "test_inc_counter_unknown", 30, false);
    assert_null(c2);
    // expected success
    c2 = umc_get_set(umc, "test_inc_counter", 30, false);
    // w/locks
    c2 = umc_get_set(umc, "test_inc_counter", 30, true);

    // set to lower value (not allowed for INCREMENTAL)
    umc_set(c, 5);
    assert_int_equal(c->values.last.value, 30);
    assert_int_equal(c->values.max, 30);

    // free
    umc_free_ctx(umc);
}

static void
umc_gauge_cntr_test(void **state)
{
    // create ctx
    umc_ctx_t *umc = umc_new_ctx();
    assert_non_null(umc);

    // new gauge counter
    umc_t *c = umc_new_counter(umc, "test_gauge_counter", UMCT_GAUGE);
    assert_non_null(c);
    assert_string_equal(c->idp, "test_gauge_counter");
    assert_int_equal(c->type, UMCT_GAUGE);

    // last value
    assert_int_equal(c->values.last.value, 0);
    assert_int_equal(c->values.max, 0);

    // inc
    umc_inc(c, 10);
    assert_int_equal(c->values.last.value, 10);
    assert_int_equal(c->values.max, 10);

    // check get/set/inc for GAUGE
    // expected failure
    umc_t *c2 = umc_get_inc(umc, "test_gauge_counter_unknown", 10, false);
    assert_null(c2);
    // expected success
    c2 = umc_get_inc(umc, "test_gauge_counter", 10, false);

    // check umc_set
    // expected failure
    c2 = umc_get_set(umc, "test_gauge_counter_unknown", 30, false);
    assert_null(c2);
    // expected success
    c2 = umc_get_set(umc, "test_gauge_counter", 30, false);

    // set to lower value (allowed for GAUGE)
    umc_set(c, 5);
    // max value should not have changed
    assert_int_equal(c->values.last.value, 5);
    assert_int_equal(c->values.max, 30);

    // free
    umc_free_ctx(umc);
}

static void
match_cb(umc_t *c, void *arg)
{
    int *cntr = arg;
    (*cntr)++;
}

static void
umc_match_test(void **state)
{
    // create ctx
    umc_ctx_t *umc = umc_new_ctx();
    assert_non_null(umc);

    // create 10 counters
    char perf_id[UMC_NAME_MAX];
    for (int i = 0; i < 10; i++) {
        // new gauge counter
        snprintf(perf_id, sizeof(perf_id), "test_%d", i);
        umc_t *c = umc_new_counter(umc, perf_id, UMCT_GAUGE);
        assert_non_null(c);
        assert_string_equal(c->idp, perf_id);
        assert_int_equal(c->type, UMCT_GAUGE);
    }

    // match
    int cntr = 0;
    umc_match(umc, "test_*", false, &match_cb, &cntr);
    // expect 10 matched counters
    assert_int_equal(cntr, 10);

    // free
    umc_free_ctx(umc);
}

int
main(int argc, char **argv)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(umc_create_free_test),
        cmocka_unit_test(umc_create_cntr_test),
        cmocka_unit_test(umc_inc_cntr_test),
        cmocka_unit_test(umc_gauge_cntr_test),
        cmocka_unit_test(umc_match_test),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
