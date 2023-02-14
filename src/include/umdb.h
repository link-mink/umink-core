/*
 *               _____  ____ __
 *   __ ____ _  /  _/ |/ / //_/
 *  / // /  ' \_/ //    / ,<
 *  \_,_/_/_/_/___/_/|_/_/|_|
 *
 * SPDX-License-Identifier: MIT
 *
 */

#ifndef UMDB
#define UMDB

#include <sqlite3.h>
#include <stdbool.h>
#include <stddef.h>

// types
typedef struct umdb_mngr_d umdb_mngrd_t;
typedef struct umdb_uauth_d umdb_uauth_d_t;

/**
 * Query type
 */
enum query_type
{
    /** User authentication */
    USER_AUTH = 0,
    /** Add new user */
    USER_ADD,
    /** Delete user */
    USER_DEL,
    /** Delete user */
    USER_CMD_DEL,
    /** Authenticate user */
    USER_CMD_AUTH,
    /** Custom authentication */
    USER_CMD_SPECIFIC_AUTH,
    /** Get user */
    USER_GET
};

/** DB descriptor */
struct umdb_mngr_d {
    /** sqlite db pointer */
    sqlite3 *db;
};

/** User auth-result descriptor */
struct umdb_uauth_d {
    /** Authentication result */
    int auth;
    /** User id */
    int id;
    /** User flags */
    int flags;
    /** User name */
    const char *usr;
};

/**
 * Create new DB manager
 *
 * @param[in]   db  Database name
 * @param[in]   mem In-memory flag
 *
 * @return      New DB manager
 */
umdb_mngrd_t *umdb_mngr_new(const char *db, bool mem);

/**
 * Free DB manager
 *
 * @param[in]   m   DB manager
 */
void umdb_mngr_free(umdb_mngrd_t *m);

/**
 * Authenticate user
 *
 * @param[in]   m   DB manager
 * @param[out]  res User authentication result
 * @param[in]   u   User name
 * @param[in]   p   User password
 *
 * @return      0 for success or error code
 */
int umdb_mngr_uauth(umdb_mngrd_t *m,
                    umdb_uauth_d_t *res,
                    const char *u,
                    const char *p);

/**
 * Get user info
 *
 * @param[in]   m   DB manager
 * @param[out]  res User data result
 * @param[in]   u   User name
 *
 * @return      0 for success or error code
 */
int umdb_mngr_uget(umdb_mngrd_t *m, umdb_uauth_d_t *res, const char *u);

/**
 * Init custom user storage
 *
 * @param[in]   m   DB manager
 * @param[in]   n   User DB name
 *
 * @return      0 for success or error code
 */
int umdb_mngr_store_init(umdb_mngrd_t *m, const char *n);

/**
 * Set custom storage data
 *
 * @param[in]   m   DB manager
 * @param[in]   db  User DB name
 * @param[in]   k   Data key
 * @param[in]   v   Data value
 *
 * @return      0 for success or error code
 */
int umdb_mngr_store_set(umdb_mngrd_t *m,
                        const char *db,
                        const char *k,
                        const char *v);

/**
 * Get custom storage data
 *
 * @param[in]   m       DB manager
 * @param[in]   db      User DB name
 * @param[in]   k       Data key
 * @param[out]  out     Output buffer
 * @param[out]  out_sz  Output buffer size
 *
 * @return      0 for success or error code
 */
int umdb_mngr_store_get(umdb_mngrd_t *m,
                        const char *db,
                        const char *k,
                        char **out,
                        size_t *out_sz);

#endif /* ifndef UMDB */
