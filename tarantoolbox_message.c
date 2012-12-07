#include "tarantoolbox_private.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

typedef struct __attribute__ ((__packed__)) {
    uint32_t error;
    uint8_t data[];
} tarantoolbox_message_response_pack_t;

tarantoolbox_message_t *tarantoolbox_message_init(tarantoolbox_message_type_t type, void *data, size_t size) {
    tarantoolbox_message_t *message = malloc(sizeof(*message));
    memset(message, 0, sizeof(*message));
    message->type = type;
    message->response.error = ERR_CODE_REQUEST_IN_PROGRESS;
    message->message = iproto_message_init(type, data, size);
    message->data = data;
    if (type != SELECT) {
        iproto_message_opts_t *opts = iproto_message_options(message->message);
        opts->from = FROM_MASTER;
    }
    return message;
}

void tarantoolbox_message_free(tarantoolbox_message_t *message) {
    iproto_message_free(message->message);
    if (message->response.tuples)
        tarantoolbox_tuples_free(message->response.tuples);
    free(message->data);
    free(message);
}

iproto_message_t *tarantoolbox_message_get_iproto_message(tarantoolbox_message_t *message) {
    return message->message;
}

tarantoolbox_message_type_t tarantoolbox_message_type(tarantoolbox_message_t *message) {
    return message->type;
}

static void tarantoolbox_affected_unpack(tarantoolbox_message_t *message, void *data, size_t size) {
    if (message->want_result) {
        message->response.tuples = tarantoolbox_tuples_unpack(data, size, NULL);
        if (message->response.tuples == NULL)
            message->response.error = ERR_CODE_INVALID_RESPONSE;
    } else {
        if (size < sizeof(uint32_t)) {
            message->response.error = ERR_CODE_INVALID_RESPONSE;
            return;
        }
        message->response.tuples = tarantoolbox_tuples_init(*(uint32_t *)data, false);
    }
}

void tarantoolbox_message_unpack(tarantoolbox_message_t *message) {
    if (message->response.unpacked) return;
    message->response.error = iproto_message_error(message->message);
    message->response.error_string = iproto_error_string(message->response.error);
    if (message->response.error != ERR_CODE_OK) {
        message->response.unpacked = true;
        return;
    }
    size_t size;
    void *data = iproto_message_response(message->message, &size, &message->response.replica);
    if (size < sizeof(tarantoolbox_message_response_pack_t)) {
        tarantoolbox_log(LOG_ERROR, "response is too short to contain error code");
        message->response.error = ERR_CODE_INVALID_RESPONSE;
        message->response.unpacked = true;
        return;
    }
    tarantoolbox_message_response_pack_t *pack = (tarantoolbox_message_response_pack_t *)data;
    message->response.error = pack->error;
    data = pack->data;
    size -= sizeof(tarantoolbox_message_response_pack_t);
    if (message->response.error == ERR_CODE_OK) {
        tarantoolbox_affected_unpack(message, data, size);
    } else if (size > 0) {
        message->response.error_string = data;
    }
    message->response.unpacked = true;
}

tarantoolbox_error_t tarantoolbox_message_error(tarantoolbox_message_t *message, char **error_string) {
    tarantoolbox_message_unpack(message);
    if (error_string)
        *error_string = message->response.error_string;
    return message->response.error;
}

tarantoolbox_tuples_t *tarantoolbox_message_response(tarantoolbox_message_t *message, bool *replica) {
    tarantoolbox_message_unpack(message);
    if (replica)
        *replica = message->response.replica;
    return message->response.tuples;
}
