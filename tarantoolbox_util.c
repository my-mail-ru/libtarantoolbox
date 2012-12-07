#include "tarantoolbox_private.h"

tarantoolbox_logmask_t tarantoolbox_logmask = LOG_INFO;

void tarantoolbox_set_logmask(tarantoolbox_logmask_t mask) {
    tarantoolbox_logmask = mask;
}
