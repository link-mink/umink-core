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
#include <umink_plugin.h>
#include <umatomic.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <umdaemon.h>
#include <time.h>
#include <json_object.h>
#include <json_tokener.h>
#include <spscq.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <libubus.h>
#include <libubox/blob.h>
#include <libubox/blobmsg.h>
#include <libubox/blobmsg_json.h>
#include <poll.h>
#include <umlua.h>

/*************/
/* Plugin ID */
/*************/
static const char *PLG_ID = "plg_sysagent_openwrt.so";

/**********************************************/
/* list of command implemented by this plugin */
/**********************************************/
int COMMANDS[] = {
    // end of list marker
    -1
};

// globals
static struct ubus_context *uctx;
static pthread_t ubus_th;
static struct blob_buf b;
static umplg_mngr_t *umplgm;

// list_signals policiy
static const struct blobmsg_policy list_signals_policy[] = {};

// run_signal policy order
enum {
    RUN_SIGNAL_ID,
    RUN_SIGNAL_ARGS,
    RUN_SIGNAL_AUTH,
    __RUN_SIGNAL_MAX
};

// run_signal policiy
static const struct blobmsg_policy run_signal_policy[] = {
    [RUN_SIGNAL_ID] = { .name = "id", .type = BLOBMSG_TYPE_STRING },
    [RUN_SIGNAL_ARGS] = { .name = "args", .type = BLOBMSG_TYPE_STRING },
    [RUN_SIGNAL_AUTH] = { .name = "auth", .type = BLOBMSG_TYPE_STRING }
};

// match umsignal callback
static void
signal_match_cb(umplg_sh_t *shd, void *args)
{
    // skip special signals
    if (shd->id[0] == '@') {
        return;
    }
    void *json_list = blobmsg_open_table(&b, NULL);
    blobmsg_add_string(&b, "name", shd->id);
    // get lua env
    struct lua_env_d **env = utarray_eltptr(shd->args, 1);
    blobmsg_add_string(&b, "path", ((*env)->path));
    blobmsg_add_u32 (&b, "interval", ((*env)->interval));
    blobmsg_add_u8(&b, "auto_start", UM_ATOMIC_GET(&(*env)->active));
    blobmsg_close_table(&b, json_list);
}

// run signal method
static int
run_signal(struct ubus_context *ctx,
           struct ubus_object *obj,
           struct ubus_request_data *req,
           const char *method,
           struct blob_attr *msg)
{
    blob_buf_init(&b, 0);
    struct blob_attr *tb[__RUN_SIGNAL_MAX];

    blobmsg_parse(run_signal_policy,
                  __RUN_SIGNAL_MAX,
                  tb,
                  blob_data(msg),
                  blob_len(msg));

    if (tb[RUN_SIGNAL_ID] != NULL) {
        // id and args
        char *id = blobmsg_get_string(tb[RUN_SIGNAL_ID]);
        char *args = "";
        char *auth = "";
        int uflags = 0;
        // check args
        if (tb[RUN_SIGNAL_ARGS] != NULL) {
            args = blobmsg_get_string(tb[RUN_SIGNAL_ARGS]);
        }
        // check auth
        if (tb[RUN_SIGNAL_AUTH] != NULL) {
            auth = blobmsg_get_string(tb[RUN_SIGNAL_AUTH]);
            // check auth (already checked for errors)
            if (auth != NULL) {
                json_object *j = json_tokener_parse(auth);
                if (j != NULL) {
                    json_object *j_usr = json_object_object_get(j, "flags");
                    uflags = json_object_get_int(j_usr);
                    json_object_put(j);
                }
            }
        }

        // output buffer
        char *buff = NULL;
        size_t b_sz = 0;

        // input data
        umplg_data_std_t e_d = { .items = NULL };
        umplg_data_std_items_t items = { .table = NULL };
        umplg_data_std_item_t item = { .name = "", .value = args };
        umplg_data_std_item_t auth_item = { .name = "", .value = auth };
        // init std data
        umplg_stdd_init(&e_d);
        umplg_stdd_item_add(&items, &item);
        umplg_stdd_item_add(&items, &auth_item);
        umplg_stdd_items_add(&e_d, &items);

        // run signal (set)
        int r = umplg_proc_signal(umplgm, id, &e_d, &buff, &b_sz, uflags, NULL);

        switch (r) {
            case UMPLG_RES_SUCCESS:
                if (buff && !blobmsg_add_json_from_string(&b, buff)) {
                    blobmsg_add_string(&b, "result", buff);
                }
                break;

            case UMPLG_RES_AUTH_ERROR:
                blobmsg_add_string(&b, "result", "authentication error");
                break;

            case UMPLG_RES_UNKNOWN_SIGNAL:
                blobmsg_add_string(&b, "result", "unknown signal");
                break;

            default:
                blobmsg_add_string(&b, "result", "unknown error");
                break;
        }
        HASH_CLEAR(hh, items.table);
        umplg_stdd_free(&e_d);
        free(buff);

    // missing args
    } else {
        blobmsg_add_string(&b, "result", "missing arguments");
    }

    return ubus_send_reply(ctx, req, b.head);
}

// list signals method
static int
list_signals(struct ubus_context *ctx,
             struct ubus_object *obj,
             struct ubus_request_data *req,
             const char *method,
             struct blob_attr *msg)
{

    blob_buf_init(&b, 0);
    void *json_uri = blobmsg_open_array(&b, "signals");

    // init target/function specific lua modules
    umplg_match_signal(umplgm, "*", &signal_match_cb, NULL);

    blobmsg_close_array(&b, json_uri);

    return ubus_send_reply(ctx, req, b.head);
}

// ubus object methods
static const struct ubus_method umink_methods[] = {
    UBUS_METHOD("list_signals", &list_signals, list_signals_policy),
    UBUS_METHOD("run_signal", &run_signal, run_signal_policy),
};

// ubus object type
static struct ubus_object_type umink_obj_type =
    UBUS_OBJECT_TYPE("signals", umink_methods);

// umink ubus object
static struct ubus_object umink_object = {
    .name = "umink",
    .type = &umink_obj_type,
    .methods = umink_methods,
    .n_methods = ARRAY_SIZE(umink_methods),

};

// ubus thread
void *
thread_ubus(void *args)
{
    // setup ubus socket polling
    struct pollfd pfd = { .fd = uctx->sock.fd, .events = POLLIN };
    umd_log(UMD, UMD_LLT_INFO, "plg_openwrt: [ubus thread starting");

    while (!umd_is_terminating()) {
        // poll ubus socket
        int r = poll(&pfd, 1, 1000);

        // critical error
        if (r == -1) {
            umd_log(UMD,
                    UMD_LLT_ERROR,
                    "plg_openwrt: [ubus socket error (%s)]",
                    strerror(errno));
            break;

        // socket inactive
        } else if (pfd.revents == 0) {
            continue;

        // data or error pending
        } else if (pfd.revents & POLLIN) {
            ubus_handle_event(uctx);

        // socket error POLLERR/POLLHUP
        } else {
            umd_log(UMD, UMD_LLT_ERROR, "plg_openwrt: [ubus socket error]");
            break;
        }
    }

    umd_log(UMD, UMD_LLT_INFO, "plg_openwrt: [ubus thread terminating");
    return NULL;
}

/****************/
/* init handler */
/****************/
int
init(umplg_mngr_t *pm, umplgd_t *pd)
{
    umplgm = pm;
    signal(SIGPIPE, SIG_IGN);
    uctx = ubus_connect(NULL);
    if (uctx == NULL) {
        umd_log(UMD, UMD_LLT_ERROR, "plg_openwrt: [cannot connect to ubusd]");
        return 1;
    }
    int r = ubus_add_object(uctx, &umink_object);
    if (r) {
        umd_log(UMD,
                UMD_LLT_ERROR,
                "plg_openwrt: [cannot add umink object, %s]",
                ubus_strerror(r));
        ubus_free(uctx);
        uctx = NULL;
        return 1;
    }
    pthread_create(&ubus_th, NULL, &thread_ubus, NULL);

    return 0;
}

/*********************/
/* terminate handler */
/*********************/
static void
term_phase_0(umplg_mngr_t *pm, umplgd_t *pd)
{
    if (uctx != NULL) {
        pthread_join(ubus_th, NULL);
    }
}

static void
term_phase_1(umplg_mngr_t *pm, umplgd_t *pd)
{
    if (uctx != NULL) {
        ubus_free(uctx);
    }
    blob_buf_free(&b);
}

int
terminate(umplg_mngr_t *pm, umplgd_t *pd, int phase)
{
    switch (phase) {
    case 0:
        term_phase_0(pm, pd);
        break;
    case 1:
        term_phase_1(pm, pd);
        break;
    default:
        break;
    }
    return 0;
}


/*************************/
/* local command handler */
/*************************/
// GCOVR_EXCL_START
int
run_local(umplg_mngr_t *pm, umplgd_t *pd, int cmd_id, umplg_idata_t *data)
{
    // not used
    return 0;
}

/*******************/
/* command handler */
/*******************/
int
run(umplg_mngr_t *pm, umplgd_t *pd, int cmd_id, umplg_idata_t *data)
{
    // not used
    return 0;
}
// GCOVR_EXCL_STOP
