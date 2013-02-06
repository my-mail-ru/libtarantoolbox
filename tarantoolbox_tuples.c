#include "tarantoolbox_private.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

typedef struct __attribute__ (( __packed__ )) {
    uint32_t count;
    uint8_t data[];
} tarantoolbox_tuples_pack_t;

tarantoolbox_tuples_t *tarantoolbox_tuples_init(uint32_t count, bool has_tuples) {
    tarantoolbox_tuples_t *tuples = malloc(sizeof(*tuples));
    tuples->count = count;
    if (has_tuples) {
        tuples->tuples = malloc(count * sizeof(tarantoolbox_tuple_t *));
        memset(tuples->tuples, 0, count * sizeof(tarantoolbox_tuple_t *));
    } else {
        tuples->tuples = NULL;
    }
    return tuples;
}

void tarantoolbox_tuples_free(tarantoolbox_tuples_t *tuples) {
    if (!tuples) return;
    if (tuples->tuples) {
        for (uint32_t i = 0; i < tuples->count; i++)
            tarantoolbox_tuple_free(tuples->tuples[i]);
        free(tuples->tuples);
    }
    free(tuples);
}

bool tarantoolbox_tuples_has_tuples(tarantoolbox_tuples_t *tuples) {
    return tuples->tuples != NULL;
}

uint32_t tarantoolbox_tuples_get_count(tarantoolbox_tuples_t *tuples) {
    return tuples->count;
}

void tarantoolbox_tuples_set_tuple(tarantoolbox_tuples_t *tuples, uint32_t id, tarantoolbox_tuple_t *tuple) {
    assert(id < tuples->count && tuples->tuples);
    if (tuples->tuples[id])
        free(tuples->tuples[id]);
    tuples->tuples[id] = tuple;
}

tarantoolbox_tuple_t *tarantoolbox_tuples_get_tuple(tarantoolbox_tuples_t *tuples, uint32_t id) {
    assert(id < tuples->count && tuples->tuples);
    return tuples->tuples[id];
}

size_t tarantoolbox_tuples_packed_size(tarantoolbox_tuples_t *tuples) {
    size_t size = sizeof(tarantoolbox_tuples_pack_t);
    for (uint32_t i = 0; i < tuples->count; i++)
        size += tarantoolbox_tuple_packed_size(tuples->tuples[i]);
    return size;
}

size_t tarantoolbox_tuples_pack(tarantoolbox_tuples_t *tuples, void *data) {
    tarantoolbox_tuples_pack_t *pack = (tarantoolbox_tuples_pack_t *)data;
    pack->count = tuples->count;
    void *buf = pack->data;
    for (uint32_t i = 0; i < tuples->count; i++)
        buf += tarantoolbox_tuple_pack(tuples->tuples[i], buf);
    return buf - data;
}

tarantoolbox_tuples_t *tarantoolbox_tuples_unpack(void *data, size_t size, size_t *tsize) {
    void *data0 = data;
    if (size < sizeof(tarantoolbox_tuples_pack_t)) {
        tarantoolbox_log(LOG_ERROR, "response is too short to contain tuples");
        return NULL;
    }
    tarantoolbox_tuples_pack_t *pack = (tarantoolbox_tuples_pack_t *)data;
    data = pack->data;
    size -= sizeof(tarantoolbox_tuples_pack_t);
    tarantoolbox_tuples_t *tuples = tarantoolbox_tuples_init(pack->count, true);
    for (uint32_t i = 0; i < pack->count; i++) {
        size_t tsize;
        tarantoolbox_tuple_t *tuple = tarantoolbox_tuple_unpack(data, size, &tsize);
        if (tuple == NULL) {
            tarantoolbox_log(LOG_ERROR, "failed to unpack tuple");
            free(tuples);
            return NULL;
        }
        tuples->tuples[i] = tuple;
        data += tsize;
        size -= tsize;
    }
    tarantoolbox_log(LOG_DEBUG, "%d tuples unpacked", pack->count);
    if (tsize)
        *tsize = data - data0;
    return tuples;
}
