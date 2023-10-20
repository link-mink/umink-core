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
set_nullptr_counter(void **state)
{
    umc_set(NULL, 10);
}

static void
get_counter_nullptr_umc(void **state)
{
    umc_t *c = umc_get(NULL, "test_counter", false);
    assert_null(c);
    // w/locks
    c = umc_get(NULL, "test_counter", true);
    assert_null(c);
}

static void
get_counter_nullptr_id(void **state)
{
    // create ctx
    umc_ctx_t *umc = umc_new_ctx();
    assert_non_null(umc);

    umc_t *c = umc_get(umc, NULL, false);
    assert_null(c);
    // w/locks
    c = umc_get(umc, NULL, true);
    assert_null(c);

    // free
    umc_free_ctx(umc);
}

static void
get_counter_increment_nullptr_umc(void **state)
{
    umc_t *c = umc_get_inc(NULL, "test_counter", 10, false);
    assert_null(c);
    // w/locks
    c = umc_get_inc(NULL, "test_counter", 10, true);
    assert_null(c);
}

static void
get_counter_increment_nullptr_id(void **state)
{
    // create ctx
    umc_ctx_t *umc = umc_new_ctx();
    assert_non_null(umc);

    umc_t *c = umc_get_inc(umc, NULL, 10, false);
    assert_null(c);
    // w/locks
    c = umc_get_inc(umc, NULL, 10, true);
    assert_null(c);

    // free
    umc_free_ctx(umc);
}

static void
increment_counter_nullptr_umc(void **state)
{
    umc_inc(NULL, 10);
}

static void
get_set_counter_nullptr_umc(void **state)
{
    // create ctx
    umc_ctx_t *umc = umc_new_ctx();
    assert_non_null(umc);

    // new incremental counter
    umc_t *c = umc_new_counter(umc, "test_inc_counter", UMCT_INCREMENTAL);
    assert_non_null(c);

    // get/set
    umc_t *c2 = umc_get_set(NULL, "test_inc_counter", 10, false);
    assert_null(c2);
    assert_int_equal(c->values.last.value, 0);
    // w/locks
    c2 = umc_get_set(NULL, "test_inc_counter", 10, true);
    assert_null(c2);
    assert_int_equal(c->values.last.value, 0);

    // free
    umc_free_ctx(umc);
}

static void
get_set_counter_nullptr_id(void **state)
{
    // create ctx
    umc_ctx_t *umc = umc_new_ctx();
    assert_non_null(umc);

    // new incremental counter
    umc_t *c = umc_new_counter(umc, "test_inc_counter", UMCT_INCREMENTAL);
    assert_non_null(c);

    // get/set
    umc_t *c2 = umc_get_set(umc, NULL, 10, false);
    assert_null(c2);
    assert_int_equal(c->values.last.value, 0);
    // w/locks
    c2 = umc_get_set(umc, NULL, 10, true);
    assert_null(c2);
    assert_int_equal(c->values.last.value, 0);

    // free
    umc_free_ctx(umc);
}


static void
set_counter_nullptr_umc(void **state)
{
    // create ctx
    umc_ctx_t *umc = umc_new_ctx();
    assert_non_null(umc);

    // new incremental counter
    umc_t *c = umc_new_counter(umc, "test_inc_counter", UMCT_INCREMENTAL);
    assert_non_null(c);

    // get/set
    umc_t *c2 = umc_get_set(NULL, "test_inc_counter", 10, false);
    assert_null(c2);
    assert_int_equal(c->values.last.value, 0);
    // w/locks
    c2 = umc_get_set(NULL, "test_inc_counter", 10, true);
    assert_null(c2);
    assert_int_equal(c->values.last.value, 0);

    // free
    umc_free_ctx(umc);

}

static void
set_counter_nullptr_id(void **state)
{
    // create ctx
    umc_ctx_t *umc = umc_new_ctx();
    assert_non_null(umc);

    // new incremental counter
    umc_t *c = umc_new_counter(umc, "test_inc_counter", UMCT_INCREMENTAL);
    assert_non_null(c);

    // get/set
    umc_t *c2 = umc_get_set(umc, NULL, 10, false);
    assert_null(c2);
    assert_int_equal(c->values.last.value, 0);
    // w/locks
    c2 = umc_get_set(umc, NULL, 10, true);
    assert_null(c2);
    assert_int_equal(c->values.last.value, 0);

    // free
    umc_free_ctx(umc);

}

static void
create_umc(void **state)
{
    // create ctx
    umc_ctx_t *umc = umc_new_ctx();
    assert_non_null(umc);

    // free
    umc_free_ctx(umc);
}

static void
free_nullptr_umc(void **state)
{
    // free
    umc_free_ctx(NULL);
}

static void
create_incremental_counter(void **state)
{
    // create ctx
    umc_ctx_t *umc = umc_new_ctx();
    assert_non_null(umc);

    // new incremental counter
    umc_t *c = umc_new_counter(umc, "test_inc_counter", UMCT_INCREMENTAL);
    assert_non_null(c);
    assert_string_equal(c->idp, "test_inc_counter");
    assert_int_equal(c->type, UMCT_INCREMENTAL);

    // free
    umc_free_ctx(umc);
}

static void
create_gauge_counter(void **state)
{
    // create ctx
    umc_ctx_t *umc = umc_new_ctx();
    assert_non_null(umc);

    // new gauge counter
    umc_t *c = umc_new_counter(umc, "test_gauge_counter", UMCT_GAUGE);
    assert_non_null(c);
    assert_string_equal(c->idp, "test_gauge_counter");
    assert_int_equal(c->type, UMCT_GAUGE);

    // free
    umc_free_ctx(umc);
}

static void
create_counter_nullptr_umc(void **state)
{
    // create ctx
    umc_ctx_t *umc = umc_new_ctx();
    assert_non_null(umc);

    // new incremental counter
    umc_t *c = umc_new_counter(NULL, "test_inc_counter", UMCT_INCREMENTAL);
    assert_null(c);

    // free
    umc_free_ctx(umc);
}

static void
increment_incremental_counter_w_set(void **state)
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
    umc_set(c, 10);
    assert_int_equal(c->values.last.value, 10);
    assert_int_equal(c->values.max, 10);
    // w/locks
    umc_get_inc(umc, "test_inc_counter", 5, true);
    assert_non_null(c);
    assert_int_equal(c->values.last.value, 15);

    // free
    umc_free_ctx(umc);
}

static void
increment_incremental_counter_w_inc(void **state)
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
    // w/locks
    umc_get_inc(umc, "test_inc_counter", 5, true);
    assert_non_null(c);
    assert_int_equal(c->values.last.value, 15);

    // free
    umc_free_ctx(umc);
}

static void
increment_unknown_incremental_counter(void **state)
{
    // create ctx
    umc_ctx_t *umc = umc_new_ctx();
    assert_non_null(umc);

    // new incremental counter
    umc_t *c = umc_new_counter(umc, "test_inc_counter", UMCT_INCREMENTAL);
    assert_non_null(c);
    assert_string_equal(c->idp, "test_inc_counter");
    assert_int_equal(c->type, UMCT_INCREMENTAL);

    // check get/set/inc for INCREMENTAL
    // expected failure
    umc_t *c2 = umc_get_inc(umc, "test_inc_counter_unknown", 10, false);
    assert_null(c2);
    // w/locks
    c2 = umc_get_inc(umc, "test_inc_counter_unknown", 10, true);
    assert_null(c2);

    // free
    umc_free_ctx(umc);
}

static void
set_gauge_counter_w_set(void **state)
{
    // create ctx
    umc_ctx_t *umc = umc_new_ctx();
    assert_non_null(umc);

    // new counter
    umc_t *c = umc_new_counter(umc, "test_gauge_counter", UMCT_GAUGE);
    assert_non_null(c);
    assert_string_equal(c->idp, "test_gauge_counter");
    assert_int_equal(c->type, UMCT_GAUGE);

    // check get/set/inc for GAUGE
    // expected failure
    umc_set(c, 10);
    assert_non_null(c);
    assert_int_equal(c->values.last.value, 10);

    // free
    umc_free_ctx(umc);
}


static void
set_gauge_counter_w_get_set(void **state)
{
    // create ctx
    umc_ctx_t *umc = umc_new_ctx();
    assert_non_null(umc);

    // new counter
    umc_t *c = umc_new_counter(umc, "test_gauge_counter", UMCT_GAUGE);
    assert_non_null(c);
    assert_string_equal(c->idp, "test_gauge_counter");
    assert_int_equal(c->type, UMCT_GAUGE);

    // check get/set/inc for GAUGE
    // expected failure
    umc_t *c2 = umc_get_set(umc, "test_gauge_counter", 10, false);
    assert_non_null(c2);
    assert_int_equal(c->values.last.value, 10);
    // w/locks
    c2 = umc_get_set(umc, "test_gauge_counter", 5, true);
    assert_non_null(c2);
    assert_int_equal(c->values.last.value, 5);

    // free
    umc_free_ctx(umc);
}

static void
increment_gauge_counter(void **state)
{
    // create ctx
    umc_ctx_t *umc = umc_new_ctx();
    assert_non_null(umc);

    // new counter
    umc_t *c = umc_new_counter(umc, "test_gauge_counter", UMCT_GAUGE);
    assert_non_null(c);
    assert_string_equal(c->idp, "test_gauge_counter");
    assert_int_equal(c->type, UMCT_GAUGE);

    // check get/set/inc for INCREMENTAL
    // expected failure
    c = umc_get_inc(umc, "test_gauge_counter", 10, false);
    assert_non_null(c);
    assert_int_equal(c->values.last.value, 10);
    // w/locks
    c = umc_get_inc(umc, "test_gauge_counter", 10, true);
    assert_non_null(c);
    assert_int_equal(c->values.last.value, 20);

    // free
    umc_free_ctx(umc);
}


static void
set_unknown_gauge_counter(void **state)
{
    // create ctx
    umc_ctx_t *umc = umc_new_ctx();
    assert_non_null(umc);

    // new incremental counter
    umc_t *c = umc_new_counter(umc, "test_gauge_counter", UMCT_GAUGE);
    assert_non_null(c);
    assert_string_equal(c->idp, "test_gauge_counter");
    assert_int_equal(c->type, UMCT_GAUGE);

    // check get/set/inc for GAUGE
    // expected failure
    umc_t *c2 = umc_get_set(umc, "test_gauge_counter_unknown", 10, false);
    assert_null(c2);
    // w/locks
    c2 = umc_get_set(umc, "test_gauge_counter_unknown", 10, true);
    assert_null(c2);

    // free
    umc_free_ctx(umc);
}

static void
decrement_incremental_counter_w_set(void **state)
{
    // create ctx
    umc_ctx_t *umc = umc_new_ctx();
    assert_non_null(umc);

    // new incremental counter
    umc_t *c = umc_new_counter(umc, "test_inc_counter", UMCT_INCREMENTAL);
    assert_non_null(c);
    assert_string_equal(c->idp, "test_inc_counter");
    assert_int_equal(c->type, UMCT_INCREMENTAL);

    // initial value
    umc_t *c2 = umc_get_inc(umc, "test_inc_counter", 10, false);
    assert_non_null(c2);
    assert_int_equal(c->values.last.value, 10);

    // value should remain at 10
    umc_set(c, 5);
    assert_non_null(c);
    assert_int_equal(c->values.last.value, 10);

    // free
    umc_free_ctx(umc);
}

static void
decrement_incremental_counter_w_get_set(void **state)
{
    // create ctx
    umc_ctx_t *umc = umc_new_ctx();
    assert_non_null(umc);

    // new incremental counter
    umc_t *c = umc_new_counter(umc, "test_inc_counter", UMCT_INCREMENTAL);
    assert_non_null(c);
    assert_string_equal(c->idp, "test_inc_counter");
    assert_int_equal(c->type, UMCT_INCREMENTAL);

    // initial value
    umc_t *c2 = umc_get_inc(umc, "test_inc_counter", 10, false);
    assert_non_null(c2);
    assert_int_equal(c->values.last.value, 10);

    // value should remain at 10
    c2 = umc_get_set(umc, "test_inc_counter", 5, false);
    assert_non_null(c2);
    assert_int_equal(c->values.last.value, 10);

    // w/locks
    c2 = umc_get_set(umc, "test_inc_counter", 5, true);
    assert_non_null(c2);
    assert_int_equal(c->values.last.value, 10);

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
counter_match(void **state)
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
        cmocka_unit_test(create_umc),
        cmocka_unit_test(free_nullptr_umc),
        cmocka_unit_test(create_incremental_counter),
        cmocka_unit_test(create_gauge_counter),
        cmocka_unit_test(create_counter_nullptr_umc),
        cmocka_unit_test(increment_incremental_counter_w_inc),
        cmocka_unit_test(increment_incremental_counter_w_set),
        cmocka_unit_test(increment_unknown_incremental_counter),
        cmocka_unit_test(decrement_incremental_counter_w_get_set),
        cmocka_unit_test(decrement_incremental_counter_w_set),
        cmocka_unit_test(set_gauge_counter_w_set),
        cmocka_unit_test(set_gauge_counter_w_get_set),
        cmocka_unit_test(set_unknown_gauge_counter),
        cmocka_unit_test(increment_gauge_counter),
        cmocka_unit_test(get_counter_nullptr_umc),
        cmocka_unit_test(get_counter_nullptr_id),
        cmocka_unit_test(get_counter_increment_nullptr_umc),
        cmocka_unit_test(get_counter_increment_nullptr_id),
        cmocka_unit_test(increment_counter_nullptr_umc),
        cmocka_unit_test(get_set_counter_nullptr_umc),
        cmocka_unit_test(get_set_counter_nullptr_id),
        cmocka_unit_test(set_counter_nullptr_umc),
        cmocka_unit_test(set_counter_nullptr_id),
        cmocka_unit_test(set_nullptr_counter),
        cmocka_unit_test(counter_match),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
