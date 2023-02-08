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

umcounter_ctx_t *
umcounter_new_ctx()
{
    umcounter_ctx_t *ctx = calloc(1, sizeof(umcounter_ctx_t));
    pthread_mutex_init(&ctx->mtx, NULL);
    return ctx;
}

void
umcounter_free_ctx(umcounter_ctx_t *ctx)
{
    if (ctx == NULL) {
        return;
    }

    umcounter_t *c;
    umcounter_t *tmp;

    // loop counters
    pthread_mutex_lock(&ctx->mtx);
    HASH_ITER(hh, ctx->counters, c, tmp)
    {
        HASH_DEL(ctx->counters, c);
        pthread_mutex_destroy(&c->mtx);
        free(c);
    }
    pthread_mutex_unlock(&ctx->mtx);
    pthread_mutex_destroy(&ctx->mtx);
    free(ctx);
}

umcounter_t *
umcounter_new_counter(umcounter_ctx_t *ctx,
                      const char *id,
                      enum umcounter_type type)
{
    if (ctx == NULL || id == NULL) {
        return NULL;
    }

    // check for duplicates
    umcounter_t *c = NULL;

    pthread_mutex_lock(&ctx->mtx);

    HASH_FIND_STR(ctx->counters, id, c);
    if (c != NULL) {
        pthread_mutex_unlock(&ctx->mtx);
        return NULL;
    }

    // create counter
    c = calloc(1, sizeof(umcounter_t));
    snprintf(c->id, sizeof(c->id), "%s", id);
    c->type = type;
    pthread_mutex_init(&c->mtx, NULL);
    HASH_ADD_STR(ctx->counters, id, c);

    pthread_mutex_unlock(&ctx->mtx);

    return c;
}

umcounter_t *
umcounter_get(umcounter_ctx_t *ctx, const char *id, bool lock)
{
    if (ctx == NULL || id == NULL) {
        return NULL;
    }

    umcounter_t *c = NULL;
    if (lock) {
        pthread_mutex_lock(&ctx->mtx);
    }

    HASH_FIND_STR(ctx->counters, id, c);

    if (lock) {
        pthread_mutex_unlock(&ctx->mtx);
    }

    return c;
}

static void
update_value(umcounter_t *c, uint64_t val)
{
    // get previous value
    uint64_t pv = c->values.last.value;
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
umcounter_inc(umcounter_t *c, uint64_t val)
{
    if (c == NULL) {
        return;
    }

    pthread_mutex_lock(&c->mtx);
    update_value(c, c->values.last.value + val);
    pthread_mutex_unlock(&c->mtx);
}

umcounter_t *
umcounter_get_inc(umcounter_ctx_t *ctx, const char *id, uint64_t val, bool lock)
{
    if (lock) {
        pthread_mutex_lock(&ctx->mtx);
    }

    umcounter_t *c = umcounter_get(ctx, id, false);
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
umcounter_set(umcounter_t *c, uint64_t val)
{
    if (c != NULL) {
        pthread_mutex_lock(&c->mtx);
        update_value(c, val);
        pthread_mutex_unlock(&c->mtx);
    }
}

umcounter_t *
umcounter_get_set(umcounter_ctx_t *ctx, const char *id, uint64_t val, bool lock)
{
    if (ctx == NULL || id == NULL) {
        return NULL;
    }

    if (lock) {
        pthread_mutex_lock(&ctx->mtx);
    }

    // get counter
    umcounter_t *c = umcounter_get(ctx, id, false);

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
umcounter_match(umcounter_ctx_t *ctx,
                const char *ptrn,
                bool lock,
                umcounter_cb_t cb,
                void *arg)
{
    umcounter_t *c;
    umcounter_t *tmp;

    if (lock) {
        pthread_mutex_lock(&ctx->mtx);
    }

    // loop counters
    HASH_ITER(hh, ctx->counters, c, tmp)
    {
        // match counter
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
umcounter_get_rate(umcounter_t *c)
{

    pthread_mutex_lock(&c->mtx);
    uint64_t tdiff = c->values.ts_diff;
    uint64_t delta = c->values.delta;
    pthread_mutex_unlock(&c->mtx);

    if (tdiff > 0 && delta > 0 && c->type == UMCT_INCREMENTAL) {
        return delta / (double)tdiff * 1000000000;
    }

    return 0;
}
