/*
 *               _____  ____ __
 *   __ ____ _  /  _/ |/ / //_/
 *  / // /  ' \_/ //    / ,<
 *  \_,_/_/_/_/___/_/|_/_/|_|
 *
 * SPDX-License-Identifier: MIT
 *
 */

#ifndef UMCOUNTERS_H
#define UMCOUNTERS_H

#include <inttypes.h>
#include <time.h>
#include <uthash.h>
#include <umatomic.h>
#include <stdbool.h>
#include <pthread.h>

// consts
#define UMCOUNTER_NAME_MAX 256

// types
typedef struct umcounter_ctx umcounter_ctx_t;
typedef struct umcounter umcounter_t;
typedef struct umcounter_val umcounter_val_t;

/**
 * Counter callback
 *
 * @param[in]   c   Counter object
 * @param[in]   arg User data
 */
typedef void (*umcounter_cb_t)(umcounter_t *c, void *arg);

/**
 * Counter type
 */
enum umcounter_type {
    /** Incremental */
    UMCT_INCREMENTAL = 0,
    /** Gauge (no rate) */
    UMCT_GAUGE = 1
};

/**
 * Couter value descriptor
 */
struct umcounter_val {
    /** Counter value */
    uint64_t value;
    /** Counter timestamp */
    uint64_t ts_nsec;
};

/**
 * Counter descriptor
 */
struct umcounter {
    /** Id string */
    char id[UMCOUNTER_NAME_MAX];
    /** Counter type */
    enum umcounter_type type;
    /** Values */
    struct {
        /** Last value */
        umcounter_val_t last;
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
 * Counter context
 */
struct umcounter_ctx {
    /** Counter map */
    umcounter_t *counters;
    /** Mutex */
    pthread_mutex_t mtx;
};

/**
 * Create new counter context
 *
 * @return New counter context
 */
umcounter_ctx_t *umcounter_new_ctx();

/**
 * Free counter context
 *
 * @param[in]   Counter context
 */
void umcounter_free_ctx(umcounter_ctx_t *ctx);

/**
 * Create new counter
 *
 * @param[in]   ctx     Counter context
 * @param[in]   id      New counter id
 * @param[in]   type    Counter type
 *
 * @return      New counter object pointer
 */
umcounter_t *umcounter_new_counter(umcounter_ctx_t *ctx,
                                   const char *id,
                                   enum umcounter_type type);

/**
 * Get counter object
 *
 * @param[in]   ctx     Counter context
 * @param[in]   id      Counter id
 * @param[in]   lock    Lock mutex
 *
 * @return      Counter object pointer
 */
umcounter_t *umcounter_get(umcounter_ctx_t *ctx, const char *id, bool lock);

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
umcounter_t *umcounter_get_inc(umcounter_ctx_t *ctx,
                               const char *id,
                               uint64_t val,
                               bool lock);

/**
 * Increment counter value
 *
 * @param[in]   c       Counter object
 * @param[in]   val     Counter increment value
 */
void umcounter_inc(umcounter_t *c, uint64_t val);

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
umcounter_t *umcounter_get_set(umcounter_ctx_t *ctx,
                               const char *id,
                               uint64_t val,
                               bool lock);

/**
 * Set counter value
 *
 * @param[in]   c       Counter object
 * @param[in]   val     New counter value
 */
void umcounter_set(umcounter_t *c, uint64_t val);

/**
 * Match counter id by using the wildcard pattern
 *
 * @param[in]   ctx     Counter context
 * @param[in]   ptrn    String pattern
 * @param[in]   lock    Lock mutex
 * @param[in]   cb      Callback function to call if matched
 * @param[in]   arg     User data to pass to callback function
 */
int umcounter_match(umcounter_ctx_t *ctx,
                    const char *ptrn,
                    bool lock,
                    umcounter_cb_t cb,
                    void *arg);

/**
 * Calculate rate value (per-second value)
 *
 * @param[in]   c       Counter object
 *
 * @return      Counter rate value
 */
double umcounter_get_rate(umcounter_t *c);

#endif /* ifndef UMCOUNTERS_H */
