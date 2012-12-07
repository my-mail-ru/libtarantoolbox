#include "tarantoolbox.h"

#include <assert.h>
#include <stdlib.h>

#define COUNT 10000
#define CHUNK 1
#define SELECT_CHUNK 1

int main(int argc, char *argv[]) {
    iproto_t *iproto = iproto_init();
    iproto_shard_t *shard = iproto_shard_init();
    iproto_server_t *server = iproto_server_init("188.93.61.208", 30000);
    iproto_shard_add_servers(shard, false, &server, 1);
    iproto_server_free(server);
    iproto_add_shard(iproto, shard);

    uint32_t id = 1000011658;
    for (int i = 0; i < COUNT; i++) {
        tarantoolbox_message_t *messages[CHUNK];
        iproto_message_t *imessages[CHUNK];
        for (int j = 0; j < CHUNK; j++) {
            tarantoolbox_tuples_t *keys = tarantoolbox_tuples_init(SELECT_CHUNK, true);
            for (int k = 0; k < SELECT_CHUNK; k++) {
                tarantoolbox_tuple_t *key = tarantoolbox_tuple_init(1);
                tarantoolbox_tuple_set_field(key, 0, &id, sizeof(id));
                tarantoolbox_tuples_set_tuple(keys, k, key);
            }
            messages[j] = tarantoolbox_select_init(22, 0, keys, 0, 0);
            imessages[j] = tarantoolbox_message_get_iproto_message(messages[j]);
            tarantoolbox_tuples_free(keys);
        }
        iproto_bulk(iproto, imessages, CHUNK, NULL);
        for (int j = 0; j < CHUNK; j++) {
            tarantoolbox_error_t error = tarantoolbox_message_error(messages[j], NULL);
            assert(error == ERR_CODE_OK);
            tarantoolbox_tuples_t *tuples = tarantoolbox_message_response(messages[j], NULL);
            assert(tarantoolbox_tuples_get_count(tuples) == SELECT_CHUNK);
            for (int k = 0; k < SELECT_CHUNK; k++) {
                tarantoolbox_tuple_t *tuple = tarantoolbox_tuples_get_tuple(tuples, k);
                size_t size;
                void *data = tarantoolbox_tuple_get_field(tuple, 0, &size);
                assert(size == sizeof(uint32_t) && *(uint32_t *)data == id);
            }
            tarantoolbox_message_free(messages[j]);
        }
    }

    iproto_free(iproto);
}
