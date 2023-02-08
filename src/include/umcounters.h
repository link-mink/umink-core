/*
 *               _____  ____ __
 *   __ ____ _  /  _/ |/ / //_/
 *  / // /  ' \_/ //    / ,<
 *  \_,_/_/_/_/___/_/|_/_/|_|
 *
 * SPDX-License-Identifier: MIT
 *
 */

#ifndef UMC_H
#define UMC_H

#include <inttypes.h>
#include <time.h>
#include <uthash.h>
#include <umatomic.h>
#include <stdbool.h>
#include <pthread.h>

// consts
#define UMC_NAME_MAX 256

// types
typedef struct umc_ctx umc_ctx_t;
typedef struct umc umc_t;
typedef struct umc_val umc_val_t;
typedef struct umc_lag umc_lag_t;

/**
 * Counter callback
 *
 * @param[in]   c   Counter object
 * @param[in]   arg User data
 */
typedef void (*umc_cb_t)(umc_t *c, void *arg);

/**
 * Counter type
 */
enum umc_type
{
    /** Incremental */
    UMCT_INCREMENTAL = 0,
    /** Gauge (no rate) */
    UMCT_GAUGE = 1
};

/**
 * Counter value descriptor
 */
struct umc_val {
    /** Counter value */
    uint64_t value;
    /** Counter timestamp */
    uint64_t ts_nsec;
};

/**
 * Counter descriptor
 */
struct umc {
    /** Id string buffer */
    char id[UMC_NAME_MAX];
    /** id string pointer */
    char *idp;
    /** Counter type */
    enum umc_type type;
    /** Values */
    struct {
        /** Last value */
        umc_val_t last;
        /** Maximum value */
        uint64_t max;
        /** Time diff in nsec */
        uint64_t ts_diff;
        /** Value delta */
        int64_t delta;
    } values;
    /** Mutex */
    pthread_mutex_t mtx;

    UT_hash_handle hh;
};

/**
 * Lag measurement descriptor
 */
struct umc_lag {
    /** Start timestamp in nsec */
    uint64_t ts_start;
    /** End timestamp in nsec */
    uint64_t ts_end;
    /** Calculated lag in nsec */
    uint64_t ts_diff;
};

/**
 * Counter context
 */
struct umc_ctx {
    /** Counter map */
    umc_t *counters;
    /** Mutex */
    pthread_mutex_t mtx;
};

/**
 * Create new counter context
 *
 * @return New counter context
 */
umc_ctx_t *umc_new_ctx();

/**
 * Free counter context
 *
 * @param[in]   Counter context
 */
void umc_free_ctx(umc_ctx_t *ctx);

/**
 * Create new counter
 *
 * @param[in]   ctx     Counter context
 * @param[in]   id      New counter id
 * @param[in]   type    Counter type
 *
 * @return      New counter object or the one already
 *              bound to this id
 */
umc_t *umc_new_counter(umc_ctx_t *ctx, const char *id, enum umc_type type);

/**
 * Get counter object
 *
 * @param[in]   ctx     Counter context
 * @param[in]   id      Counter id
 * @param[in]   lock    Lock mutex
 *
 * @return      Counter object pointer
 */
umc_t *umc_get(umc_ctx_t *ctx, const char *id, bool lock);

/**
 * Get and increment Increment counter value
 *
 * @param[in]   ctx     Counter context
 * @param[in]   id      Counter id
 * @param[in]   val     Counter increment value
 * @param[in]   lock    Lock mutex
 *
 * @return      Counter object pointer
 */
umc_t *umc_get_inc(umc_ctx_t *ctx, const char *id, uint64_t val, bool lock);

/**
 * Increment counter value
 *
 * @param[in]   c       Counter object
 * @param[in]   val     Counter increment value
 */
void umc_inc(umc_t *c, uint64_t val);

/**
 * Get counter and and set value
 *
 * @param[in]   ctx     Counter context
 * @param[in]   id      Counter id
 * @param[in]   val     New counter value
 * @param[in]   lock    Lock mutex
 *
 * @return      Counter object pointer
 */
umc_t *umc_get_set(umc_ctx_t *ctx, const char *id, uint64_t val, bool lock);

/**
 * Set counter value
 *
 * @param[in]   c       Counter object
 * @param[in]   val     New counter value
 */
void umc_set(umc_t *c, uint64_t val);

/**
 * Match counter id by using the wildcard pattern
 *
 * @param[in]   ctx     Counter context
 * @param[in]   ptrn    String pattern
 * @param[in]   lock    Lock mutex
 * @param[in]   cb      Callback function to call if matched
 * @param[in]   arg     User data to pass to callback function
 */
int
umc_match(umc_ctx_t *ctx, const char *ptrn, bool lock, umc_cb_t cb, void *arg);

/**
 * Calculate rate value (per-second value)
 *
 * @param[in]   c       Counter object
 * @param[in]   lock    Lock mutex
 *
 * @return      Counter rate value
 */
double umc_get_rate(umc_t *c, bool lock);

/**
 * Start lag measurement
 *
 * @param[out]  lag     Lag descriptor
 */
void umc_lag_start(umc_lag_t *lag);

/**
 * Finish lag measurement
 *
 * @param[out]  lag     Lag descriptor
 */
void umc_lag_end(umc_lag_t *lag);

#endif /* ifndef UMC_H */
