#ifndef TARANTOOLBOX_H_INCLUDED
#define TARANTOOLBOX_H_INCLUDED

#ifndef ERR_CODES_ENUM
    #define ERR_CODES_ENUM tarantoolbox_error
    #include <iprotocluster.h>
    #undef ERR_CODES_ENUM
#else
    #include <iprotocluster.h>
#endif
#include <iproto_def.h>

#define TARANTOOLBOX_ERR_CODE_FLAG ((0x40 << 16) | IPROTO_ERR_CODE_FLAG)
#define TARANTOOLBOX_ERROR_CODES(_) \
    _(ERR_CODE_INVALID_REQUEST,   (0x01 << 16) | (TARANTOOLBOX_ERR_CODE_FLAG | FATAL_ERR_CODE_FLAG), "invalid request") \
    _(ERR_CODE_INVALID_RESPONSE,  (0x02 << 16) | (TARANTOOLBOX_ERR_CODE_FLAG | FATAL_ERR_CODE_FLAG), "invalid response")
#define TARANTOOLBOX_ALL_ERROR_CODES(x) ERROR_CODES(x) TARANTOOLBOX_ERROR_CODES(x) IPROTO_ERROR_CODES(x) LIBIPROTO_ERROR_CODES(x)
#ifndef ERR_CODES_ENUM
typedef enum tarantoolbox_error ENUM_INITIALIZER(TARANTOOLBOX_ALL_ERROR_CODES) tarantoolbox_error_t;
#else
typedef enum ERR_CODES_ENUM tarantoolbox_error_t;
#endif
#define tarantoolbox_error_string(e) iproto_error_string(e)

#define TARANTOOLBOX_LOGMASK(_) \
    _(LOG_TUPLE, (0x000001 << 8))
typedef enum tarantoolbox_logmask ENUM_INITIALIZER(TARANTOOLBOX_LOGMASK) tarantoolbox_logmask_t;

#define TARANTOOLBOX_MESSAGE_TYPE(_) \
    _(NOP, 1) \
    _(INSERT, 13) \
    _(SELECT_LIMIT, 15) \
    _(SELECT, 17) \
    _(UPDATE_FIELDS, 19) \
    _(DELETE, 20) \
    _(EXEC_LUA, 22) \
    _(PAXOS_LEADER, 90) \
    _(SELECT_KEYS, 99)
typedef enum tarantoolbox_message_type ENUM_INITIALIZER(TARANTOOLBOX_MESSAGE_TYPE) tarantoolbox_message_type_t;

typedef enum {
    WANT_RESULT    = 0x01,
    INSERT_ADD     = 0x02,
    INSERT_REPLACE = 0x04,
} tarantoolbox_flag_t;

typedef enum {
    UPDATE_OP_SET = 0,
    UPDATE_OP_ADD = 1,
    UPDATE_OP_AND = 2,
    UPDATE_OP_XOR = 3,
    UPDATE_OP_OR = 4,
    UPDATE_OP_SPLICE = 5
} tarantoolbox_update_op_type_t;

typedef struct tarantoolbox_tuple tarantoolbox_tuple_t;
typedef struct tarantoolbox_tuples tarantoolbox_tuples_t;
typedef struct tarantoolbox_message tarantoolbox_message_t;
typedef struct tarantoolbox_update_ops tarantoolbox_update_ops_t;
typedef struct tarantoolbox tarantoolbox_t;

void tarantoolbox_initialize(void);
void tarantoolbox_set_logmask(tarantoolbox_logmask_t mask);

tarantoolbox_message_t *tarantoolbox_select_init(uint32_t namespace, uint32_t index, tarantoolbox_tuples_t *keys, uint32_t offset, uint32_t limit);
tarantoolbox_message_t *tarantoolbox_insert_init(uint32_t namespace, tarantoolbox_tuple_t *tuple, uint32_t flags);
tarantoolbox_message_t *tarantoolbox_update_init(uint32_t namespace, tarantoolbox_tuple_t *key, tarantoolbox_update_ops_t *ops, uint32_t flags);
tarantoolbox_message_t *tarantoolbox_delete_init(uint32_t namespace, tarantoolbox_tuple_t *key, uint32_t flags);
void tarantoolbox_message_free(tarantoolbox_message_t *message);
iproto_message_t *tarantoolbox_message_get_iproto_message(tarantoolbox_message_t *message);
tarantoolbox_message_type_t tarantoolbox_message_type(tarantoolbox_message_t *message);
tarantoolbox_error_t tarantoolbox_message_error(tarantoolbox_message_t *message, char **error_string);
tarantoolbox_tuples_t *tarantoolbox_message_response(tarantoolbox_message_t *message, bool *replica);

tarantoolbox_update_ops_t *tarantoolbox_update_ops_init(uint32_t extend_count);
void tarantoolbox_update_ops_free(tarantoolbox_update_ops_t *ops);
void tarantoolbox_update_ops_add_op(tarantoolbox_update_ops_t *ops, uint32_t field_num, uint8_t op, void *data, size_t size);
void tarantoolbox_update_ops_add_splice(tarantoolbox_update_ops_t *ops, uint32_t field_num, int32_t offset, int32_t length, void *data, size_t size);

tarantoolbox_tuples_t *tarantoolbox_tuples_init(uint32_t count, bool has_tuples);
void tarantoolbox_tuples_free(tarantoolbox_tuples_t *tuples);
uint32_t tarantoolbox_tuples_get_count(tarantoolbox_tuples_t *tuples);
bool tarantoolbox_tuples_has_tuples(tarantoolbox_tuples_t *tuples);
void tarantoolbox_tuples_set_tuple(tarantoolbox_tuples_t *tuples, uint32_t id, tarantoolbox_tuple_t *tuple);
tarantoolbox_tuple_t *tarantoolbox_tuples_get_tuple(tarantoolbox_tuples_t *tuples, uint32_t id);

tarantoolbox_tuple_t *tarantoolbox_tuple_init(uint32_t cardinality);
void tarantoolbox_tuple_free(tarantoolbox_tuple_t *tuple);
uint32_t tarantoolbox_tuple_get_cardinality(tarantoolbox_tuple_t *tuple);
void tarantoolbox_tuple_set_field(tarantoolbox_tuple_t *tuple, uint32_t id, void *data, size_t size);
void *tarantoolbox_tuple_get_field(tarantoolbox_tuple_t *tuple, uint32_t id, size_t *size);

#endif
