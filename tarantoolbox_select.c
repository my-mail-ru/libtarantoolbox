#include "tarantoolbox_private.h"

#include <stdlib.h>

typedef struct __attribute__ ((__packed__)) {
    uint32_t namespace;
    uint32_t index;
    uint32_t offset;
    uint32_t limit;
    uint8_t data[];
} tarantoolbox_select_request_t;

tarantoolbox_message_t *tarantoolbox_select_init(uint32_t namespace, uint32_t index, tarantoolbox_tuples_t *keys, uint32_t offset, uint32_t limit) {
    size_t size = sizeof(tarantoolbox_select_request_t) + tarantoolbox_tuples_packed_size(keys);
    void *data = malloc(size);
    tarantoolbox_select_request_t *req = (tarantoolbox_select_request_t *)data;
    req->namespace = namespace;
    req->index = index;
    req->offset = offset;
    req->limit = limit ? limit : keys->count;
    tarantoolbox_tuples_pack(keys, req->data);
    tarantoolbox_message_t *message = tarantoolbox_message_init(SELECT, data, size);
    message->want_result = true;
    return message;
}
