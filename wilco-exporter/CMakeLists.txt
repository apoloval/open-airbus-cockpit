cmake_minimum_required(VERSION 2.8)

project(wilco-exporter)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/../cmake/modules/")

find_package(Boost REQUIRED)
find_package(SimConnect REQUIRED)
find_package(FSUIPC_Internal REQUIRED)

set(WE_HEADERS
)

set(WE_SOURCES
   WilcoExporter.def
   src/buffer.cpp
   src/dllmain.cpp
   src/fsuipc.cpp
   src/logging.cpp
   src/mapping.cpp
   src/main.cpp
   src/wilco.cpp
)

add_definitions(-DUNICODE)

include_directories(
   include
   ${Boost_INCLUDE_DIR}
   ${SIM_CONNECT_INCLUDE_DIR}
   ${FSUIPC_INTERNAL_INCLUDE_DIR}
   )

link_directories(
   )

add_library(WilcoExporter SHARED ${WE_SOURCES})

set_target_properties(WilcoExporter PROPERTIES
   COMPILE_FLAGS ""
   LINK_FLAGS ""
)
target_link_libraries(WilcoExporter
   ${SIM_CONNECT_LIBRARY}
   ${FSUIPC_INTERNAL_LIBRARY}
)
