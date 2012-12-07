#include "tarantoolbox_private.h"

#include <stdlib.h>

typedef struct __attribute__ ((__packed__)) {
    uint32_t namespace;
    uint8_t key[];
} tarantoolbox_delete20_request_t;

typedef struct __attribute__ ((__packed__)) {
    uint32_t namespace;
    uint32_t flags;
    uint8_t key[];
} tarantoolbox_delete21_request_t;

tarantoolbox_message_t *tarantoolbox_delete_init(uint32_t namespace, tarantoolbox_tuple_t *key, uint32_t flags) {
    size_t size = flags ? sizeof(tarantoolbox_delete21_request_t) : sizeof(tarantoolbox_delete20_request_t);
    size += tarantoolbox_tuple_packed_size(key);
    void *data = malloc(size);
    if (flags) {
        tarantoolbox_delete21_request_t *req = (tarantoolbox_delete21_request_t *)data;
        req->namespace = namespace;
        req->flags = flags;
        tarantoolbox_tuple_pack(key, req->key);
    } else {
        tarantoolbox_delete20_request_t *req = (tarantoolbox_delete20_request_t *)data;
        req->namespace = namespace;
        tarantoolbox_tuple_pack(key, req->key);
    }
    tarantoolbox_message_t *message = tarantoolbox_message_init(flags ? 21 : DELETE, data, size);
    if (flags & WANT_RESULT)
        message->want_result = true;
    return message;
}
