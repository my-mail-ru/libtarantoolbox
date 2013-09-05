cmake_minimum_required(VERSION 2.6)

project(tarantoolbox)

set(tarantoolbox_cflags "-std=gnu99 -Wall -Werror -ggdb3")

set(tarantoolbox_sources tarantoolbox.c
tarantoolbox_message.c tarantoolbox_select.c
tarantoolbox_insert.c tarantoolbox_update.c tarantoolbox_delete.c
tarantoolbox_tuples.c tarantoolbox_tuple.c tarantoolbox_field.c
tarantoolbox_call.c tarantoolbox_util.c)

add_library(tarantoolbox ${tarantoolbox_sources})
target_link_libraries(tarantoolbox iprotocluster)
set_target_properties(tarantoolbox PROPERTIES COMPILE_FLAGS "${tarantoolbox_cflags}")

add_executable(tarantoolboxtest test.c)
target_link_libraries(tarantoolboxtest tarantoolbox)
set_target_properties(tarantoolboxtest PROPERTIES COMPILE_FLAGS "${tarantoolbox_cflags}")

install(TARGETS tarantoolbox
    LIBRARY DESTINATION ${LIB_INSTALL_DIR}
    ARCHIVE DESTINATION ${LIB_INSTALL_DIR}
)
install(FILES tarantoolbox.h DESTINATION include)
