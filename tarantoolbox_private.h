#ifndef TARANTOOLBOX_PRIVATE_H_INCLUDED
#define TARANTOOLBOX_PRIVATE_H_INCLUDED

#include "tarantoolbox.h"
#include <iproto_util.h>

struct tarantoolbox_message {
    tarantoolbox_message_type_t type;
    void *data;
    iproto_message_t *message;
    bool want_result;
    struct {
        bool unpacked;
        bool replica;
        uint32_t error;
        const char *error_string;
        bool error_string_allocated;
        tarantoolbox_tuples_t *tuples;
    } response;
};

typedef struct tarantoolbox_field {
    void *data;
    size_t size;
} tarantoolbox_field_t;

struct tarantoolbox_tuple {
    uint32_t cardinality;
    tarantoolbox_field_t *fields;
};

struct tarantoolbox_tuples {
    uint32_t count;
    tarantoolbox_tuple_t **tuples;
};

tarantoolbox_message_t *tarantoolbox_message_init(tarantoolbox_message_type_t type, void *data, size_t size);

size_t tarantoolbox_tuples_packed_size(tarantoolbox_tuples_t *tuples);
size_t tarantoolbox_tuples_pack(tarantoolbox_tuples_t *tuples, void *data);
tarantoolbox_tuples_t *tarantoolbox_tuples_unpack(void *data, size_t size, size_t *tsize);

size_t tarantoolbox_tuple_packed_size(tarantoolbox_tuple_t *tuple);
size_t tarantoolbox_tuple_pack(tarantoolbox_tuple_t *tuple, void *data);
tarantoolbox_tuple_t *tarantoolbox_tuple_unpack(void *data, size_t maxsize, size_t *tsize);

size_t tarantoolbox_field_packed_size(tarantoolbox_field_t *field);
size_t tarantoolbox_field_pack(tarantoolbox_field_t *field, void *buf);
size_t tarantoolbox_field_unpack(tarantoolbox_field_t *field, void *data, size_t maxsize);

extern tarantoolbox_logmask_t tarantoolbox_logmask;

#define tarantoolbox_is_loggable(mask) \
    (((mask) & LOG_LEVEL) <= (tarantoolbox_logmask & LOG_LEVEL) || ((mask) & tarantoolbox_logmask & LOG_TYPE) != 0)

#define tarantoolbox_log(mask, format, ...) \
    if (tarantoolbox_is_loggable(mask)) \
        iproto_util_log(mask, "tarantoolbox: " format, ##__VA_ARGS__)

#define tarantoolbox_log_data(mask, data, length, format, ...) \
    if (tarantoolbox_is_loggable(mask)) \
        iproto_util_log_data(mask, data, length, "tarantoolbox: " format, ##__VA_ARGS__)

#endif
