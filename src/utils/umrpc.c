/*
 *               _____  ____ __
 *   __ ____ _  /  _/ |/ / //_/
 *  / // /  ' \_/ //    / ,<
 *  \_,_/_/_/_/___/_/|_/_/|_|
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include <umrpc.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

umrpc_ctx_t *
umrpc_new_ctx(const char *dst_addr, uint16_t dst_port)
{
    // new context
    umrpc_ctx_t *ctx = calloc(1, sizeof(umrpc_ctx_t));

    // coap context
    ctx->coap.ctx = coap_new_context(NULL);
    if (ctx->coap.ctx == NULL) {
        free(ctx);
        return NULL;
    }

    // coap src
    coap_address_t src;
    coap_address_init(&src);
    src.addr.sin.sin_family = AF_INET;
    src.addr.sin.sin_port = htons(0);
    src.addr.sin.sin_addr.s_addr = inet_addr("0.0.0.0");

    // coap dst
    coap_address_t dst;
    coap_address_init(&dst);
    dst.addr.sin.sin_family = AF_INET;
    dst.addr.sin.sin_port = htons(dst_port);
    dst.addr.sin.sin_addr.s_addr = inet_addr(dst_addr);

    // coap session
    ctx->coap.session = coap_new_client_session(ctx->coap.ctx,
                                                &src,
                                                &dst, COAP_PROTO_UDP);
    if (ctx->coap.session == NULL) {
        coap_free_context(ctx->coap.ctx);
        free(ctx);
        return NULL;
    }

    return ctx;
}

void
umrpc_free_ctx(umrpc_ctx_t *ctx)
{
    if (ctx == NULL) {
        return;
    }
    coap_free_context(ctx->coap.ctx);
    free(ctx);
}

int
umrpc_send(umrpc_ctx_t *ctx, const void *data, size_t sz)
{
    // context check
    if (ctx == NULL) {
        return 1;
    }
    // buffer check
    if (data == NULL || sz == 0) {
        return 2;
    }

    const char *uri = "coap://127.0.0.1/hello";
    coap_uri_t coap_uri;
    coap_split_uri(uri, strlen(uri), &coap_uri);
    coap_pdu_t *req = coap_new_pdu(COAP_MESSAGE_CON, COAP_REQUEST_PUT, ctx->coap.session);

    uint8_t token[8];
    size_t tl = 0;
    coap_session_new_token(ctx->coap.session, &tl, token);
    if (!coap_add_token(req, tl, token)) {
        coap_log_debug("cannot add token to request\n");
    }

    coap_add_option(req, COAP_OPTION_URI_PATH, coap_uri.path.length, coap_uri.path.s);
    coap_add_data(req, sz, data);
    coap_set_log_level(LOG_DEBUG);
    coap_show_pdu(COAP_LOG_INFO, req);
    coap_send(ctx->coap.session, req);

    return 0;
}

