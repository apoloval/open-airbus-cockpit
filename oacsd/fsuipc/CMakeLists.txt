cmake_minimum_required(VERSION 2.8)

project(fsuipc)

set(fsuipc_user_HEADERS
    include/fsuipc/fsuipc_user.h
)

set(fsuipc_user_internal_SOURCES
   src/ipc-user.h
   src/module-user.c
)

set(fsuipc_user_INCLUDE_DIR
   ${CMAKE_CURRENT_SOURCE_DIR}/include
   PARENT_SCOPE)

include_directories(
   ${CMAKE_CURRENT_SOURCE_DIR}/include/fsuipc
)

add_library(fsuipc_user_internal STATIC
   ${fsuipc_user_internal_SOURCES}
   ${fsuipc_user_HEADERS})
