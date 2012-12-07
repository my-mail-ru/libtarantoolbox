#include "tarantoolbox_private.h"

#include <stdlib.h>
#include <string.h>

typedef struct __attribute__ ((__packed__)) {
    uint32_t namespace;
    uint32_t flags;
    uint8_t key[];
} tarantoolbox_update_request_t;

typedef struct __attribute__ ((__packed__)) {
    uint32_t count;
    uint8_t ops[];
} tarantoolbox_update_request_ops_t;

typedef struct __attribute__ ((__packed__)) {
    uint32_t field_num;
    uint8_t op;
    uint8_t field[];
} tarantoolbox_update_request_op_t;

struct tarantoolbox_update_ops {
    size_t size;
    size_t alloc_size;
    tarantoolbox_update_request_ops_t *data;
};

tarantoolbox_update_ops_t *tarantoolbox_update_ops_init(uint32_t extend_count) {
    tarantoolbox_update_ops_t *ops = malloc(sizeof(*ops));
    ops->size = sizeof(tarantoolbox_update_request_ops_t);
    ops->alloc_size = sizeof(tarantoolbox_update_request_ops_t) + extend_count * (sizeof(tarantoolbox_update_request_op_t) + 5);
    ops->data = malloc(ops->alloc_size);
    ops->data->count = 0;
    return ops;
}

void tarantoolbox_update_ops_free(tarantoolbox_update_ops_t *ops) {
    free(ops->data);
    free(ops);
}

static void tarantoolbox_update_ops_need(tarantoolbox_update_ops_t *ops, size_t size) {
    while (ops->size + size > ops->alloc_size) {
        ops->alloc_size *= 2;
        ops->data = realloc(ops->data, ops->alloc_size);
    }
}

void tarantoolbox_update_ops_add_op(tarantoolbox_update_ops_t *ops, uint32_t field_num, uint8_t op, void *data, size_t size) {
    tarantoolbox_field_t field;
    field.data = data;
    field.size = size;
    size_t opsize = sizeof(tarantoolbox_update_request_op_t) + tarantoolbox_field_packed_size(&field);
    tarantoolbox_update_ops_need(ops, opsize);
    tarantoolbox_update_request_op_t *opdata = (tarantoolbox_update_request_op_t *)((void *)ops->data + ops->size);
    opdata->field_num = field_num;
    opdata->op = op;
    tarantoolbox_field_pack(&field, opdata->field);
    ops->size += opsize;
    ops->data->count++;
}

void tarantoolbox_update_ops_add_splice(tarantoolbox_update_ops_t *ops, uint32_t field_num, int32_t offset, int32_t length, void *data, size_t size) {
    tarantoolbox_field_t field[3];
    field[0].data = &offset;
    field[0].size = sizeof(offset);
    field[1].data = &length;
    field[1].size = sizeof(length);
    field[2].data = data;
    field[2].size = size;
    size_t vsize = 0;
    for (int i = 0; i < 3; i++)
        vsize += tarantoolbox_field_packed_size(&field[i]);
    void *vdata = malloc(vsize);
    void *ptr = vdata;
    for (int i = 0; i < 3; i++)
        ptr += tarantoolbox_field_pack(&field[i], ptr);
    tarantoolbox_update_ops_add_op(ops, field_num, UPDATE_OP_SPLICE, vdata, vsize);
    free(vdata);
}

tarantoolbox_message_t *tarantoolbox_update_init(uint32_t namespace, tarantoolbox_tuple_t *key, tarantoolbox_update_ops_t *ops, uint32_t flags) {
    size_t size = sizeof(tarantoolbox_update_request_t) + tarantoolbox_tuple_packed_size(key) + ops->size;
    void *data = malloc(size);
    tarantoolbox_update_request_t *req = (tarantoolbox_update_request_t *)data;
    req->namespace = namespace;
    req->flags = flags;
    void *ops_data = req->key + tarantoolbox_tuple_pack(key, req->key);
    memcpy(ops_data, ops->data, ops->size);
    tarantoolbox_message_t *message = tarantoolbox_message_init(UPDATE_FIELDS, data, size);
    if (flags & WANT_RESULT)
        message->want_result = true;
    return message;
}
