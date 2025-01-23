/*
 *               _____  ____ __
 *   __ ____ _  /  _/ |/ / //_/
 *  / // /  ' \_/ //    / ,<
 *  \_,_/_/_/_/___/_/|_/_/|_|
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include <umink_pkg_config.h>
#include <string.h>
#include <mosquitto_broker.h>
#include <mosquitto.h>
#include <mosquitto_plugin.h>
#include <umdb.h>
#include <json_object.h>
#include <json_tokener.h>
#include <stdio.h>
#include <uthash.h>
#include <jwt.h>

// mink db
static char *db_name = NULL;
// user ban opts
static struct {
    uint32_t max_failed_nr;
    uint32_t failed_time_span;
    uint32_t ban_duration;
} user_ban_cfg = { 3, 60, 300 };
// db manager
static umdb_mngrd_t *dbm = NULL;
// plugin id
static mosquitto_plugin_id_t *mosq_pid = NULL;
// user failed logins
struct umuser_d {
    char *username;
    time_t first_failed_ts;
    uint32_t failed_nr;
    time_t banned_until_ts;
    UT_hash_handle hh;
};
// jwt options
static char *jwt_public_key = NULL;
static char *jwt_private_key = NULL;
static int jwt_exp = 600;
// umusers list
static struct umuser_d *umusers = NULL;

static int
cb_pld_mod(int event, void *event_data, void *userdata)
{
    struct mosquitto_evt_message *ed = event_data;

    // skip non-mink topics
    if (strncmp(ed->topic, "mink/", 5) != 0 ) {
        return MOSQ_ERR_SUCCESS;
    }

    //  get username for client
    const char *username = mosquitto_client_username(ed->client);

    // find user in db
    umdb_uauth_d_t uauth;
    if (umdb_mngr_uget(dbm, &uauth, username)) {
        mosquitto_log_printf(MOSQ_LOG_ERR,
                             "plg_mosquitto_auth: cannot authenicate user");
        return MOSQ_ERR_AUTH;
    }

    // get payload as C string
    size_t sz = ed->payloadlen + 1;
    char *buff = mosquitto_calloc(1, sz);
    memcpy(buff, ed->payload, ed->payloadlen);
    // decode as json
    json_object *j = json_tokener_parse(buff);
    if (j == NULL) {
        return MOSQ_ERR_SUCCESS;
    }

    // check for mink user info header override (mink/<DEV_UUID>/<SESSION>/res topic)
    json_object *j_musr = json_object_object_get(j, "mink_usr");
    bool skip_musr = false;
    if (j_musr != NULL) {
        json_object *j_musr_skip = json_object_object_get(j_musr, "skip");
        if (j_musr_skip != NULL &&
            json_object_is_type(j_musr_skip, json_type_boolean)) {
            // skip mink user header
            if (json_object_get_boolean(j_musr_skip) == 1) {
                skip_musr = true;
            }
        }
        // remove mink header
        json_object_object_del(j, "mink_usr");
    }

    // add mink user info
    if (!skip_musr) {
        j_musr = json_object_new_object();
        json_object *j_muflgs = json_object_new_int(uauth.flags);
        json_object *j_muid = json_object_new_int(uauth.id);
        json_object_object_add(j_musr, "uid", j_muid);
        json_object_object_add(j_musr, "flags", j_muflgs);

        // add mink user and payload
        json_object *j_mobj = json_object_new_object();
        json_object_object_add(j_mobj, "mink_usr", j_musr);
        json_object_object_add(j_mobj, "mink_pld", j);

        // create new payload
        const char *npld =
            json_object_to_json_string_ext(j_mobj, JSON_C_TO_STRING_PLAIN);
        if (npld == NULL) {
            json_object_put(j_mobj);
            return MOSQ_ERR_MALFORMED_PACKET;
        }

        // replace payload
        ed->payload = mosquitto_strdup(npld);
        ed->payloadlen = strlen(npld);

        // free
        mosquitto_free(buff);
        json_object_put(j_mobj);

        return MOSQ_ERR_SUCCESS;

    // do not include mink user info
    } else {
        // create new payload
        const char *npld =
            json_object_to_json_string_ext(j, JSON_C_TO_STRING_PLAIN);
        if (npld == NULL) {
            json_object_put(j);
            return MOSQ_ERR_MALFORMED_PACKET;
        }

        // replace payload
        ed->payload = mosquitto_strdup(npld);
        ed->payloadlen = strlen(npld);

        // free
        mosquitto_free(buff);
        json_object_put(j);

        return MOSQ_ERR_SUCCESS;
    }

    return MOSQ_ERR_SUCCESS;
}

static int
cb_acl_check(int event, void *event_data, void *userdata)
{
    struct mosquitto_evt_acl_check *ed = event_data;

    // get username for client
    const char *username = mosquitto_client_username(ed->client);
    // anonymous not allowed
    if (username == NULL) {
        return MOSQ_ERR_AUTH;
    }

    // find user in db
    umdb_uauth_d_t uauth;
    if (!umdb_mngr_uget(dbm, &uauth, username)) {
        // not found
        if (uauth.id < 1) {
            return MOSQ_ERR_AUTH;
        }
        // "admin" topic requested, check user flags
        if (strncmp(ed->topic, "mink/admin/", 11) == 0 && uauth.flags != 1) {
            return MOSQ_ERR_AUTH;
        }

    } else {
        mosquitto_log_printf(MOSQ_LOG_ERR,
                             "plg_mosquitto_auth: cannot authenicate user");
        return MOSQ_ERR_AUTH;
    }

    return MOSQ_ERR_SUCCESS;
}

static int
jwt_parse(char *data, umdb_uauth_d_t *uauth, int *iat)
{
    if (data == NULL || uauth == NULL) {
        return 1;
    }
    printf("DATA: %s\n", data);
    int res = 0;
    json_object *j = json_tokener_parse(data);
    if (j == NULL) {
        res = 1;

    } else {
        json_object *j_usr_id = json_object_object_get(j, "usr_id");
        json_object *j_usr = json_object_object_get(j, "usr");
        json_object *j_usr_flgs = json_object_object_get(j, "usr_flgs");
        json_object *j_iat = json_object_object_get(j, "iat");
        if (j_usr_id == NULL || j_usr == NULL || j_iat == NULL) {
            res = 1;

        } else {
            uauth->auth = 1;
            uauth->usr = strdup(json_object_get_string(j_usr));
            uauth->id = json_object_get_int(j_usr_id);
            uauth->flags = json_object_get_int(j_usr_flgs);
            *iat= json_object_get_int(j_iat);
        }
    }

    json_object_put(j);

    return res;
}

static int
cb_basic_auth(int event, void *event_data, void *userdata)
{
    struct mosquitto_evt_basic_auth *ed = event_data;

    // - anonymous not allowed
    // - empty password is allowed since JWT tokens are
    //   transmitted via username field
    if (ed->username == NULL) {
         return MOSQ_ERR_AUTH;
    }
    // uauth info
    umdb_uauth_d_t uauth = { .auth = 0, .flags = -1, .id = 0, .usr = NULL };
    const char *username = ed->username;
    bool use_jwt = false;

    // JWT
    jwt_valid_t *jwt_vld = NULL;
    jwt_t *jwt = NULL;

    // JWT load public key
    FILE *fp = fopen(jwt_public_key, "r");
    if (fp == NULL) {
         return MOSQ_ERR_AUTH;
    }
    unsigned char pub_key[2048];
    size_t pub_key_l = fread(pub_key, 1, sizeof(pub_key), fp);
    pub_key[pub_key_l] = '\0';
    fclose(fp);

    // JWT setup validator
    int res = jwt_valid_new(&jwt_vld, JWT_ALG_RS256);
    if (res != 0 || jwt_vld == NULL) {
         return MOSQ_ERR_AUTH;
    }
    jwt_valid_set_headers(jwt_vld, 1);
    jwt_valid_set_now(jwt_vld, time(NULL));

    // decode JWT
    res = jwt_decode(&jwt, username, pub_key, pub_key_l);
    if (res != 0 || jwt == NULL) {
         uauth.auth = 0;
         free((char *)uauth.usr);

    // useing JWT, assume successful authentication
    } else {
        int iat = 0;
         // JWT decoded, now validate it
         if (jwt_validate(jwt, jwt_vld) == 0 &&
             jwt_parse(jwt_get_grants_json(jwt, NULL), &uauth, &iat) == 0) {

            // check if expired
            time_t now = time(NULL);

            // not expired
            if (iat + jwt_exp > now) {
                use_jwt = true;
                username = uauth.usr;

            // expired
            } else {
                 uauth.auth = 0;
                 free((char *)uauth.usr);
            }
         }
    }

    // umuser list
    struct umuser_d *umuser = NULL;
    HASH_FIND_STR(umusers, username, umuser);

    // auth user (success), in case of JWT this method will always succeed
    if (!umdb_mngr_uauth(dbm, &uauth, username, ed->password)) {
        // unknown user
        if (uauth.id < 1) {
            return MOSQ_ERR_AUTH;

        // username found, create umuser in the list
        } else {
            // create user in umusers list
            if (umuser == NULL) {
                umuser = calloc(1, sizeof(struct umuser_d));
                umuser->username = strdup(username);
                // add user to the list
                HASH_ADD_KEYPTR(hh,
                                umusers,
                                umuser->username,
                                strlen(umuser->username),
                                umuser);
            }
        }
        // check if banned
        if (umuser->banned_until_ts > 0) {
            struct timespec clk;
            clock_gettime(CLOCK_MONOTONIC, &clk);
            // user still banned
            if (clk.tv_sec <= umuser->banned_until_ts) {
                res = MOSQ_ERR_AUTH;

            // user no longer banned
            }else{
                umuser->banned_until_ts = 0;
                umuser->failed_nr = 0;
                umuser->first_failed_ts = 0;
                res = MOSQ_ERR_SUCCESS;
            }
            if (use_jwt) {
                free((char *)username);
            }
            return res;
        }

        // not authenticated due to wrong password
        if (uauth.auth != 1) {
            // inc failed attempt
            struct timespec clk;
            clock_gettime(CLOCK_MONOTONIC, &clk);
            ++umuser->failed_nr;
            if (umuser->failed_nr == 1) {
                umuser->first_failed_ts = clk.tv_sec;
            }

            // reset counters if configured time span
            // has expired (time period from first failed attempt)
            if (clk.tv_sec - umuser->first_failed_ts >
                user_ban_cfg.failed_time_span) {

                umuser->failed_nr = 1;
                umuser->first_failed_ts = clk.tv_sec;
            }

            // ban user if threshold reached
            if (umuser->failed_nr >= user_ban_cfg.max_failed_nr) {
                umuser->banned_until_ts = clk.tv_sec + user_ban_cfg.ban_duration;
            }

            return MOSQ_ERR_AUTH;


        }

    } else {
        mosquitto_log_printf(MOSQ_LOG_ERR,
                             "plg_mosquitto_auth: cannot authenicate user");
        return MOSQ_ERR_AUTH;
    }

    // reset failed attempt counters
    umuser->banned_until_ts = 0;
    umuser->failed_nr = 0;
    umuser->first_failed_ts = 0;
    if (use_jwt) {
        free((char *)username);
    }

    return MOSQ_ERR_SUCCESS;
}

int
mosquitto_plugin_version(int supported_version_count,
                         const int *supported_versions)
{
    for (int i = 0; i < supported_version_count; i++) {
        if (supported_versions[i] == 5) {
            return 5;
        }
    }
    return -1;
}

int
mosquitto_plugin_init(mosquitto_plugin_id_t *identifier,
                      void **user_data,
                      struct mosquitto_opt *opts,
                      int opt_count)
{
    // process options
    for (int i = 0; i < opt_count; i++) {
        // mink db
        if (strncmp(opts[i].key, "db_name", 7) == 0) {
            db_name = opts[i].value;
        // max failed login attempts
        } else if (strncmp(opts[i].key, "max_failed_logins", 17) == 0) {
            user_ban_cfg.max_failed_nr = atoi(opts[i].value);
        // failed login check time span
        } else if (strncmp(opts[i].key, "failed_login_time_span", 22) == 0) {
            user_ban_cfg.failed_time_span = atoi(opts[i].value);
        // user ban duration
        } else if (strncmp(opts[i].key, "failed_login_ban_duration", 26) == 0) {
            user_ban_cfg.ban_duration = atoi(opts[i].value);
        // jwt public key
        } else if (strncmp(opts[i].key, "jwt_public_key", 14) == 0) {
            jwt_public_key = opts[i].value;
        // jwt private key
        } else if (strncmp(opts[i].key, "jwt_private_key", 15) == 0) {
            jwt_private_key = opts[i].value;
        // jwt expration
        } else if (strncmp(opts[i].key, "jwt_expiration", 14) == 0) {
            jwt_exp = atoi(opts[i].value);
        }

    }

    // missing db options
    if (db_name == NULL) {
        mosquitto_log_printf(MOSQ_LOG_ERR,
                             "plg_mosquitto_auth: db_name option is missing");
        return MOSQ_ERR_AUTH;
    }
    // create umdbm and connect
    dbm = umdb_mngr_new(db_name, false);
    if (dbm == NULL) {
        mosquitto_log_printf(MOSQ_LOG_ERR,
                             "plg_mosquitto_auth: error while connecting");
        return MOSQ_ERR_AUTH;
    }

    // register BASIC AUTH callback
    int rc = mosquitto_callback_register(identifier,
                                         MOSQ_EVT_BASIC_AUTH,
                                         cb_basic_auth,
                                         NULL,
                                         NULL);
    if (rc != MOSQ_ERR_SUCCESS) {
        return rc;
    }

    // register ACL CHECK callback
    rc = mosquitto_callback_register(identifier,
                                     MOSQ_EVT_ACL_CHECK,
                                     cb_acl_check,
                                     NULL,
                                     NULL);
    if (rc != MOSQ_ERR_SUCCESS) {
        return rc;
    }

    // register MESSAGE callback
    rc = mosquitto_callback_register(identifier,
                                     MOSQ_EVT_MESSAGE,
                                     cb_pld_mod,
                                     NULL,
                                     NULL);
    if (rc != MOSQ_ERR_SUCCESS) {
        return rc;
    }

    // save plg id
    mosq_pid = identifier;

    // plugin initialised
    return MOSQ_ERR_SUCCESS;
}

int
mosquitto_plugin_cleanup(void *user_data,
                         struct mosquitto_opt *opts,
                         int opt_count)
{
    if (mosq_pid) {
        mosquitto_callback_unregister(mosq_pid,
                                      MOSQ_EVT_BASIC_AUTH,
                                      cb_basic_auth,
                                      NULL);

        mosquitto_callback_unregister(mosq_pid,
                                      MOSQ_EVT_ACL_CHECK,
                                      cb_acl_check,
                                      NULL);

        mosquitto_callback_unregister(mosq_pid,
                                      MOSQ_EVT_MESSAGE,
                                      cb_pld_mod,
                                      NULL);

        umdb_mngr_free(dbm);

        struct umuser_d *umuser;
        struct umuser_d *umuser_tmp;

        HASH_ITER(hh, umusers, umuser, umuser_tmp)
        {
            HASH_DEL(umusers, umuser);
            free(umuser->username);
            free(umuser);
        }
    }

    return MOSQ_ERR_SUCCESS;
}
