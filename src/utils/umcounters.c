/*
 *               _____  ____ __
 *   __ ____ _  /  _/ |/ / //_/
 *  / // /  ' \_/ //    / ,<
 *  \_,_/_/_/_/___/_/|_/_/|_|
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include <config.h>
#include <stdio.h>
#include <umcounters.h>
#include <fnmatch.h>

#ifdef UNIT_TESTING
#include <cmocka_tests.h>
#endif

umc_ctx_t *
umc_new_ctx()
{
    umc_ctx_t *ctx = calloc(1, sizeof(umc_ctx_t));
    pthread_mutex_init(&ctx->mtx, NULL);
    return ctx;
}

void
umc_free_ctx(umc_ctx_t *ctx)
{
    if (ctx == NULL) {
        return;
    }

    umc_t *c;
    umc_t *tmp;

    // loop counters
    pthread_mutex_lock(&ctx->mtx);
    HASH_ITER(hh, ctx->counters, c, tmp)
    {
        HASH_DEL(ctx->counters, c); // GCOVR_EXCL_BR_LINE
        pthread_mutex_destroy(&c->mtx);
        free(c);
    }
    pthread_mutex_unlock(&ctx->mtx);
    pthread_mutex_destroy(&ctx->mtx);
    free(ctx);
}

umc_t *
umc_new_counter(umc_ctx_t *ctx, const char *id, enum umc_type type)
{
    if (ctx == NULL || id == NULL) {
        return NULL;
    }

    // check for duplicates
    umc_t *c = NULL;

    pthread_mutex_lock(&ctx->mtx);

    HASH_FIND_STR(ctx->counters, id, c); // GCOVR_EXCL_BR_LINE
    if (c != NULL) {
        pthread_mutex_unlock(&ctx->mtx);
        return c;
    }

    // create counter
    c = calloc(1, sizeof(umc_t));
    snprintf(c->id, sizeof(c->id), "%s", id);
    c->idp = c->id;
    c->type = type;
    pthread_mutex_init(&c->mtx, NULL);
    HASH_ADD_STR(ctx->counters, id, c); // GCOVR_EXCL_BR_LINE

    pthread_mutex_unlock(&ctx->mtx);

    return c;
}

umc_t *
umc_get(umc_ctx_t *ctx, const char *id, bool lock)
{
    if (ctx == NULL || id == NULL) {
        return NULL;
    }

    umc_t *c = NULL;
    if (lock) {
        pthread_mutex_lock(&ctx->mtx);
    }

    HASH_FIND_STR(ctx->counters, id, c); // GCOVR_EXCL_BR_LINE

    if (lock) {
        pthread_mutex_unlock(&ctx->mtx);
    }

    return c;
}

static void
update_value(umc_t *c, uint64_t val)
{
    // get previous value
    uint64_t pv = c->values.last.value;
    // lower value only available fora GAUGE counters
    if (val <= pv && c->type != UMCT_GAUGE) {
        return;
    }
    c->values.last.value = val;
    // update max value and timestamp
    c->values.max = val > pv ? val : pv;
    // rate is only used for incremental counters
    if (c->type != UMCT_INCREMENTAL) {
        return;
    }
    // get previous and current ts in nsec
    uint64_t p_ts_nsec = c->values.last.ts_nsec;
    // get current ts in nsec
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    uint64_t ts_nsec = ts.tv_sec * 1e9 + ts.tv_nsec;
    // calculate rate
    c->values.ts_diff = ts_nsec - p_ts_nsec;
    c->values.delta = val - pv;
    // update timestamp
    c->values.last.ts_nsec = ts_nsec;
}

void
umc_inc(umc_t *c, uint64_t val)
{
    if (c == NULL) {
        return;
    }

    pthread_mutex_lock(&c->mtx);
    update_value(c, c->values.last.value + val);
    pthread_mutex_unlock(&c->mtx);
}

umc_t *
umc_get_inc(umc_ctx_t *ctx, const char *id, uint64_t val, bool lock)
{
    if (lock) {
        pthread_mutex_lock(&ctx->mtx);
    }

    umc_t *c = umc_get(ctx, id, false);
    if (c != NULL) {
        pthread_mutex_lock(&c->mtx);
        update_value(c, c->values.last.value + val);
        pthread_mutex_unlock(&c->mtx);
    }

    if (lock) {
        pthread_mutex_unlock(&ctx->mtx);
    }

    return c;
}

void
umc_set(umc_t *c, uint64_t val)
{
    if (c != NULL) {
        pthread_mutex_lock(&c->mtx);
        update_value(c, val);
        pthread_mutex_unlock(&c->mtx);
    }
}

umc_t *
umc_get_set(umc_ctx_t *ctx, const char *id, uint64_t val, bool lock)
{
    if (ctx == NULL || id == NULL) {
        return NULL;
    }

    if (lock) {
        pthread_mutex_lock(&ctx->mtx);
    }

    // get counter
    umc_t *c = umc_get(ctx, id, false);

    // update rate and unupdate ts
    if (c != NULL) {
        pthread_mutex_lock(&c->mtx);
        update_value(c, val);
        pthread_mutex_unlock(&c->mtx);
    }

    if (lock) {
        pthread_mutex_unlock(&ctx->mtx);
    }

    return c;
}

int
umc_match(umc_ctx_t *ctx, const char *ptrn, bool lock, umc_cb_t cb, void *arg)
{
    umc_t *c;
    umc_t *tmp;

    if (lock) {
        pthread_mutex_lock(&ctx->mtx);
    }

    // loop counters
    HASH_ITER(hh, ctx->counters, c, tmp)
    {
        // match counter
#if !defined(FNM_EXTMATCH)
#define FNM_EXTMATCH 0
#endif
        if (fnmatch(ptrn, c->id, FNM_CASEFOLD | FNM_EXTMATCH) == 0) {
            cb(c, arg);
        }
    }

    if (lock) {
        pthread_mutex_unlock(&ctx->mtx);
    }

    return 0;
}

double
umc_get_rate(umc_t *c, bool lock)
{

    if (lock) {
        pthread_mutex_lock(&c->mtx);
    }
    uint64_t tdiff = c->values.ts_diff;
    uint64_t delta = c->values.delta;
    if (lock) {
        pthread_mutex_unlock(&c->mtx);
    }

    if (tdiff > 0 && delta > 0 && c->type == UMCT_INCREMENTAL) {
        return delta / (double)tdiff * 1000000000;
    }

    return 0;
}

void
umc_lag_start(umc_lag_t *lag)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    lag->ts_start = ts.tv_sec * 1e9 + ts.tv_nsec;
}

void
umc_lag_end(umc_lag_t *lag)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    lag->ts_end = ts.tv_sec * 1e9 + ts.tv_nsec;
    lag->ts_diff = lag->ts_end - lag->ts_start;
}

