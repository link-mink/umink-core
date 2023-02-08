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
 *
 * @return      New DB manager
 */
umdb_mngrd_t *umdb_mngr_new(const char *db);

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

#endif /* ifndef UMDB */
