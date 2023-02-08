/*
 *               _____  ____ __
 *   __ ____ _  /  _/ |/ / //_/
 *  / // /  ' \_/ //    / ,<
 *  \_,_/_/_/_/___/_/|_/_/|_|
 *
 * SPDX-License-Identifier: MIT
 *
 */

#ifndef UMRPC_H
#define UMRPC_H

#include <coap3/coap.h>

typedef struct umrpc_ctx umrpc_ctx_t;

struct umrpc_ctx {
    struct {
        coap_context_t *ctx;
        coap_session_t *session;
    } coap;

    int coap_fd;
};

umrpc_ctx_t *umrpc_new_ctx(const char *dst_addr, uint16_t dst_port);

void umrpc_free_ctx(umrpc_ctx_t *ctx);

int umrpc_send(umrpc_ctx_t *ctx, const void *data, size_t sz);

#endif /* ifndef UMRPC_H */
