#include "tarantoolbox_private.h"

#include <stdlib.h>
#include <string.h>

typedef struct __attribute__ ((__packed__)) {
    uint32_t flags;
    uint8_t data[];
} tarantoolbox_call_request_t;

tarantoolbox_message_t *tarantoolbox_call_init(const char *function, tarantoolbox_tuple_t *tuple, uint32_t flags) {
    tarantoolbox_field_t func;
    func.data = (void *)function;
    func.size = strlen(function);
    size_t size = sizeof(tarantoolbox_call_request_t) + tarantoolbox_field_packed_size(&func) + tarantoolbox_tuple_packed_size(tuple);
    void *data = malloc(size);
    tarantoolbox_call_request_t *req = (tarantoolbox_call_request_t *)data;
    req->flags = flags;
    size_t off = tarantoolbox_field_pack(&func, req->data);
    tarantoolbox_tuple_pack(tuple, req->data + off);
    tarantoolbox_message_t *message = tarantoolbox_message_init(EXEC_LUA, data, size);
    message->want_result = true;
    return message;
}
