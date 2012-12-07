#include "tarantoolbox_private.h"

#include <stdlib.h>

typedef struct __attribute__ ((__packed__)) {
    uint32_t namespace;
    uint32_t flags;
    uint8_t tuple[];
} tarantoolbox_insert_request_t;

tarantoolbox_message_t *tarantoolbox_insert_init(uint32_t namespace, tarantoolbox_tuple_t *tuple, uint32_t flags) {
    size_t size = sizeof(tarantoolbox_insert_request_t) + tarantoolbox_tuple_packed_size(tuple);
    void *data = malloc(size);
    tarantoolbox_insert_request_t *req = (tarantoolbox_insert_request_t *)data;
    req->namespace = namespace;
    req->flags = flags;
    tarantoolbox_tuple_pack(tuple, req->tuple);
    tarantoolbox_message_t *message = tarantoolbox_message_init(INSERT, data, size);
    if (flags & WANT_RESULT)
        message->want_result = true;
    return message;
}
