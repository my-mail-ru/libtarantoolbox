#include "tarantoolbox_private.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

// Here pack and unpack formats are different.
// Pack is used to pack select keys, so pack format has no size.
// Unpack is used to unpack tuples, so unpack format has size

typedef struct __attribute__ (( __packed__ )) {
    uint32_t size;
    uint32_t cardinality;
    uint8_t data[];
} tarantoolbox_tuple_pack_t;

#define tarantoolbox_tuple_log(tuple, mask, format, ...) \
    if (tarantoolbox_is_loggable(mask)) \
        _tarantoolbox_tuple_log(tuple, mask, "tarantoolbox: " format, ##__VA_ARGS__)

tarantoolbox_tuple_t *tarantoolbox_tuple_init(uint32_t cardinality) {
    tarantoolbox_tuple_t *tuple = malloc(sizeof(*tuple));
    tuple->cardinality = cardinality;
    tuple->fields = malloc(cardinality * sizeof(tarantoolbox_field_t));
    return tuple;
}

void tarantoolbox_tuple_free(tarantoolbox_tuple_t *tuple) {
    if (!tuple) return;
    free(tuple->fields);
    free(tuple);
}

uint32_t tarantoolbox_tuple_get_cardinality(tarantoolbox_tuple_t *tuple) {
    return tuple->cardinality;
}

void tarantoolbox_tuple_set_field(tarantoolbox_tuple_t *tuple, uint32_t id, void *data, size_t size) {
    assert(id < tuple->cardinality);
    tuple->fields[id].data = data;
    tuple->fields[id].size = size;
}

void *tarantoolbox_tuple_get_field(tarantoolbox_tuple_t *tuple, uint32_t id, size_t *size) {
    assert(id < tuple->cardinality);
    *size = tuple->fields[id].size;
    return tuple->fields[id].data;
}

static void _tarantoolbox_tuple_log(tarantoolbox_tuple_t *tuple, tarantoolbox_logmask_t mask, const char *format, ...) {
    va_list ap;
    va_start(ap, format);
    iproto_util_log_prefix(mask, format, ap);
    va_end(ap);
    fprintf(stderr, ": [%u]", tuple->cardinality);
    for (uint32_t i = 0; i < tuple->cardinality; i++) {
        fprintf(stderr, " [%zu]{", tuple->fields[i].size);
        for (int j = 0; j < tuple->fields[i].size; j++) {
            fprintf(stderr, " %02x", ((unsigned char *)tuple->fields[i].data)[j]);
        }
        fprintf(stderr, " }");
    }
    fprintf(stderr, "\n");
}

size_t tarantoolbox_tuple_packed_size(tarantoolbox_tuple_t *tuple) {
    size_t size = sizeof(uint32_t);
    for (uint32_t i = 0; i < tuple->cardinality; i++) {
        size += tarantoolbox_field_packed_size(&tuple->fields[i]);
    }
    return size;
}

size_t tarantoolbox_tuple_pack(tarantoolbox_tuple_t *tuple, void *data) {
    void *data0 = data;
    *(uint32_t *)data = tuple->cardinality;
    data += sizeof(uint32_t);
    for (uint32_t i = 0; i < tuple->cardinality; i++) {
        data += tarantoolbox_field_pack(&tuple->fields[i], data);
    }
    tarantoolbox_tuple_log(tuple, LOG_DEBUG | LOG_TUPLE, "tuple packed");
    return data - data0;
}

tarantoolbox_tuple_t *tarantoolbox_tuple_unpack(void *data, size_t maxsize, size_t *tsize) {
    if (maxsize < sizeof(tarantoolbox_tuple_pack_t)) {
        tarantoolbox_log(LOG_ERROR, "tuple does not contain header");
        return 0;
    }
    tarantoolbox_tuple_pack_t *pack = (tarantoolbox_tuple_pack_t *)data;
    if (pack->size > maxsize) {
        tarantoolbox_log(LOG_ERROR, "tuple is too long: %u > %u", pack->size, maxsize);
        return 0;
    }
    uint32_t size = pack->size;
    *tsize = sizeof(tarantoolbox_tuple_pack_t) + size;
    uint32_t cardinality = pack->cardinality;
    data = pack->data;
    if (cardinality > maxsize) {
        tarantoolbox_log(LOG_ERROR, "cardinality is too large");
        return 0;
    }
    tarantoolbox_tuple_t *tuple = tarantoolbox_tuple_init(cardinality);
    for (uint32_t i = 0; i < cardinality; i++) {
        size_t fsize = tarantoolbox_field_unpack(&tuple->fields[i], data, size);
        if (fsize == 0) {
            tarantoolbox_log(LOG_ERROR, "failed to unpack field");
            return 0;
        }
        data += fsize;
        size -= fsize;
    }
    if (size != 0) {
        tarantoolbox_log(LOG_ERROR, "extra data at end of tuple");
        return 0;
    }
    tarantoolbox_tuple_log(tuple, LOG_DEBUG | LOG_TUPLE, "tuple unpacked");
    return tuple;
}
