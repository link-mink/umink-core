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
#include <stdio.h>
#include <string.h>
#include <umdb.h>

/******************/
/* sql statements */
/******************/
// authenticate user
static const char *SQL_USER_AUTH =
    "SELECT a.id, "
    "       a.flags, "
    "       a.username, "
    "       iif(b.username is NULL, 0, 1) as auth "
    "FROM user a "
    "LEFT JOIN "
    "   (SELECT flags, username  "
    "    FROM user a "
    "    WHERE username = ? AND "
    "    password = ?) b "
    "ON a.username = b.username "
    "WHERE a.username = ?";

// get user
static const char *SQL_USER_GET = "SELECT * FROM user "
                                  "WHERE username = ?";

// create user storage db
static const char *SQL_STORAGE_INIT =
    "CREATE TABLE IF NOT EXISTS %s (k STRING PRIMARY KEY, v STRING)";

// set custom user data
static const char *SQL_STORAGE_SET =
    "INSERT INTO %s VALUES (?, ?) ON CONFLICT(k) DO UPDATE SET v = ?";

// get custom user data
static const char *SQL_STORAGE_GET = "SELECT v FROM %s WHERE k = ?";

umdb_mngrd_t *
umdb_mngr_new(const char *db, bool mem)
{
    if (!mem && db == NULL) {
        return NULL;
    }
    sqlite3 *db_p = NULL;
    if (!sqlite3_open_v2(db,
                         &db_p,
                         SQLITE_OPEN_READWRITE | SQLITE_OPEN_FULLMUTEX |
                             (mem ? SQLITE_OPEN_MEMORY : 0),
                         NULL)) {
        umdb_mngrd_t *m = malloc(sizeof(umdb_mngrd_t));
        m->db = db_p;
        return m;
    }

    sqlite3_close_v2(db_p);
    return NULL;
}

void
umdb_mngr_free(umdb_mngrd_t *m)
{
    if (m == NULL) {
        return;
    }
    if (m->db) {
        sqlite3_close_v2(m->db);
    }
    free(m);
}

int
umdb_mngr_uauth(umdb_mngrd_t *m,
                umdb_uauth_d_t *res,
                const char *u,
                const char *p)
{
    if (m == NULL || m->db == NULL || res == NULL || u == NULL || p == NULL) {
        return 1;
    }
    // prepare statement
    sqlite3_stmt *stmt = NULL;
    int r = sqlite3_prepare_v2(m->db, SQL_USER_AUTH, -1, &stmt, NULL);
    if (r != SQLITE_OK) {
        return 2;
    }

    // username
    if (sqlite3_bind_text(stmt, 1, u, strlen(u), SQLITE_STATIC)) {
        return 3;
    }
    // pwd
    if (sqlite3_bind_text(stmt, 2, p, strlen(p), SQLITE_STATIC)) {
        return 4;
    }
    // username
    if (sqlite3_bind_text(stmt, 3, u, strlen(u), SQLITE_STATIC)) {
        return 5;
    }

    // step
    int usr_flags = 0;
    int usr_id = -1;
    int auth = -1;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        usr_id = sqlite3_column_int(stmt, 0);
        usr_flags = sqlite3_column_int(stmt, 1);
        auth = sqlite3_column_int(stmt, 3);
    }
    // cleanup
    if (sqlite3_clear_bindings(stmt)) {
        return 6;
    }
    if (sqlite3_reset(stmt)) {
        return 7;
    }
    if (sqlite3_finalize(stmt)) {
        return 8;
    }

    // user auth result
    res->auth = auth;
    res->id = usr_id;
    res->flags = usr_flags;

    // success
    return 0;
}

int
umdb_mngr_uget(umdb_mngrd_t *m, umdb_uauth_d_t *res, const char *u)
{
    if (m == NULL || m->db == NULL || res == NULL || u == NULL) {
        return 1;
    }
    // prepare statement
    sqlite3_stmt *stmt = NULL;
    int r = sqlite3_prepare_v2(m->db, SQL_USER_GET, -1, &stmt, NULL);
    if (r != SQLITE_OK) {
        return 2;
    }

    // username
    if (sqlite3_bind_text(stmt, 1, u, strlen(u), SQLITE_STATIC)) {
        return 3;
    }

    // step
    int usr_flags = 0;
    int usr_id = -1;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        usr_id = sqlite3_column_int(stmt, 0);
        usr_flags = sqlite3_column_int(stmt, 3);
    }

    // cleanup
    if (sqlite3_clear_bindings(stmt)) {
        return 6;
    }
    if (sqlite3_reset(stmt)) {
        return 7;
    }
    if (sqlite3_finalize(stmt)) {
        return 8;
    }

    // user get result
    res->id = usr_id;
    res->flags = usr_flags;
    res->usr = u;

    // success
    return 0;
}

int
umdb_mngr_store_set(umdb_mngrd_t *m,
                    const char *db,
                    const char *k,
                    const char *v)
{
    // sanity check
    if (m == NULL || db == NULL || k == NULL || v == NULL) {
        return 1;
    }

    // prepare query
    size_t sz = snprintf(NULL, 0, SQL_STORAGE_SET, db);
    char s[sz + 1];
    snprintf(s, sz + 1, SQL_STORAGE_SET, db);

    // prepare statement
    sqlite3_stmt *stmt = NULL;
    int r = sqlite3_prepare_v2(m->db, s, -1, &stmt, NULL);
    if (r != SQLITE_OK) {
        return 2;
    }

    // key
    if (sqlite3_bind_text(stmt, 1, k, strlen(k), SQLITE_STATIC)) {
        return 3;
    }

    // value
    if (sqlite3_bind_text(stmt, 2, v, strlen(v), SQLITE_STATIC)) {
        return 4;
    }

    // updated value (on conflict)
    if (sqlite3_bind_text(stmt, 3, v, strlen(v), SQLITE_STATIC)) {
        return 5;
    }

    // step
    r = sqlite3_step(stmt);

    // cleanup
    if (sqlite3_clear_bindings(stmt)) {
        return 6;
    }
    if (sqlite3_reset(stmt)) {
        return 7;
    }
    if (sqlite3_finalize(stmt)) {
        return 8;
    }

    if (r != SQLITE_DONE){
        return 9;
    }
    return 0;
}

int
umdb_mngr_store_get(umdb_mngrd_t *m,
                    const char *db,
                    const char *k,
                    char **out,
                    size_t *out_sz)
{
    // sanity check
    if (m == NULL || db == NULL || k == NULL || out == NULL || out_sz == NULL) {
        return 1;
    }
    // output buffer check
    if (out == NULL) {
        return 2;
    }

    // prepare query
    size_t sz = snprintf(NULL, 0, SQL_STORAGE_GET, db);
    char s[sz + 1];
    snprintf(s, sz + 1, SQL_STORAGE_GET, db);

    // prepare statement
    sqlite3_stmt *stmt = NULL;
    int r = sqlite3_prepare_v2(m->db, s, -1, &stmt, NULL);
    if (r != SQLITE_OK) {
        return 3;
    }

    // key
    if (sqlite3_bind_text(stmt, 1, k, strlen(k), SQLITE_STATIC)) {
        return 4;
    }

    // get value
    r = 0;
    *out_sz = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        const unsigned char *v = sqlite3_column_text(stmt, 0);
        r = strlen((const char *)v);
        *out = strdup((const char *)v);
        *out_sz = r + 1;
    }

    // cleanup
    if (sqlite3_clear_bindings(stmt)) {
        return 5;
    }
    if (sqlite3_reset(stmt)) {
        return 6;
    }
    if (sqlite3_finalize(stmt)) {
        return 7;
    }

    return (r > 0 ? 0 : 8);
}

int
umdb_mngr_store_init(umdb_mngrd_t *m, const char *name)
{
    // sanity check
    if(m == NULL || name == NULL) {
        return 1;
    }

    // prepare query (table names cannot be changed with bind methods )
    size_t sz = snprintf(NULL, 0, SQL_STORAGE_INIT, name);
    char s[sz + 1];
    snprintf(s, sz + 1, SQL_STORAGE_INIT, name);

    // prepare statement
    sqlite3_stmt *stmt = NULL;
    int r = sqlite3_prepare_v2(m->db, s, -1, &stmt, NULL);
    if (r != SQLITE_OK) {
        return 2;
    }

    // step
    r = sqlite3_step(stmt);

    // cleanup
    if (sqlite3_clear_bindings(stmt)) {
        return 4;
    }
    if (sqlite3_reset(stmt)) {
        return 5;
    }
    if (sqlite3_finalize(stmt)) {
        return 6;
    }

    if (r != SQLITE_DONE){
        return 7;
    }
    return 0;
}
