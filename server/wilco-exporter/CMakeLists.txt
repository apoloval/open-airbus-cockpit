cmake_minimum_required(VERSION 2.8)

project(wilco-exporter)

set(wilco-exporter_SOURCES
   WilcoExporter.def
   src/dllmain.cpp
   src/main.cpp
   src/plugin.cpp
   src/wilco.cpp
)

include_directories(
   include
   ${Boost_INCLUDE_DIR}
   ${SIM_CONNECT_INCLUDE_DIR}
   ${FSUIPC_INTERNAL_INCLUDE_DIR}
   ${liboac-commons_HEADERS}
   )

add_library(WilcoExporter SHARED ${wilco-exporter_SOURCES})

set_target_properties(WilcoExporter PROPERTIES
   COMPILE_FLAGS ""
   LINK_FLAGS ""
)
target_link_libraries(WilcoExporter
   ${SIM_CONNECT_LIBRARY}
   ${FSUIPC_INTERNAL_LIBRARY}
   liboac-commons
)
