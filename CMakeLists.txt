cmake_minimum_required(VERSION 2.6)

project(tarantoolbox)

include_directories("../../shard" "/home/a.mashanov/octopus/client/libiproto")

set(tarantoolbox_cflags "-std=gnu99 -Wall -Werror -g")

set(tarantoolbox_sources tarantoolbox.c
tarantoolbox_message.c tarantoolbox_select.c
tarantoolbox_insert.c tarantoolbox_update.c tarantoolbox_delete.c
tarantoolbox_tuples.c tarantoolbox_tuple.c tarantoolbox_field.c
tarantoolbox_util.c)

add_library(tarantoolbox STATIC ${tarantoolbox_sources})
target_link_libraries(tarantoolbox "/home/a.mashanov/octopus/client/libiproto/libiproto.a")
set_target_properties(tarantoolbox PROPERTIES COMPILE_FLAGS "${tarantoolbox_cflags}")

add_executable(tarantoolboxtest test.c)
target_link_libraries(tarantoolboxtest tarantoolbox
"/home/a.mashanov/git/myutils/iproto/xs/build-iprotoshard/libiprotoshard.a" pthread m)
set_target_properties(tarantoolboxtest PROPERTIES COMPILE_FLAGS "${tarantoolbox_cflags}")
