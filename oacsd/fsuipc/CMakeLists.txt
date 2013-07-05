cmake_minimum_required(VERSION 2.8)

project(fsuipc)

set(fsuipc_user_HEADERS
    include/FSUIPC_User.h
)

set(fsuipc_user_internal_SOURCES
   src/IPCuser.h
   src/ModuleUser.c
)

include_directories(
   ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_library(fsuipc_user_internal STATIC
   ${fsuipc_user_internal_SOURCES}
   ${fsuipc_user_HEADERS})
