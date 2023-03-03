/*
 *               _____  ____ __
 *   __ ____ _  /  _/ |/ / //_/
 *  / // /  ' \_/ //    / ,<
 *  \_,_/_/_/_/___/_/|_/_/|_|
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include <string.h>
#include <mosquitto_broker.h>
#include <mosquitto.h>
#include <mosquitto_plugin.h>
#include <umdb.h>
#include <json_object.h>
#include <json_tokener.h>
#include <stdio.h>

// mink db
static char *db_name = NULL;
// db manager
static umdb_mngrd_t *dbm = NULL;
// plugin id
static mosquitto_plugin_id_t *mosq_pid = NULL;

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
cb_basic_auth(int event, void *event_data, void *userdata)
{
    struct mosquitto_evt_basic_auth *ed = event_data;

    // anonymous not allowed
    if (ed->username == NULL || ed->password == NULL) {
        return MOSQ_ERR_AUTH;
    }

    // find user in db
    umdb_uauth_d_t uauth;
    if (!umdb_mngr_uauth(dbm, &uauth, ed->username, ed->password)) {
        // not found
        if (uauth.auth != 1) {
            return MOSQ_ERR_AUTH;
        }

    } else {
        mosquitto_log_printf(MOSQ_LOG_ERR,
                             "plg_mosquitto_auth: cannot authenicate user");
        return MOSQ_ERR_AUTH;
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
        if (strncmp(opts[i].key, "db_name", 7) == 0) {
            db_name = opts[i].value;
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
    }

    return MOSQ_ERR_SUCCESS;
}
