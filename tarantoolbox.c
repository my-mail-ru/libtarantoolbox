#include "tarantoolbox_private.h"

static bool initialized = false;

void tarantoolbox_initialize(void) {
    if (initialized) return;
    iproto_initialize();
    ERRCODE_ADD(ERRCODE_DESCRIPTION, TARANTOOLBOX_ERROR_CODES);
    initialized = true;
}
