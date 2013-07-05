cmake_minimum_required(VERSION 2.8)

project(wilco-exporter)

set(wilco-exporter_SOURCES
   WilcoExporter.def
   src/dllmain.cpp
   src/main.cpp
   src/plugin.cpp
   src/wilco.cpp
   src/wilco-efis.cpp
   src/wilco-fcu.cpp
   src/wilco-internal.cpp
)

include_directories(
   include
   ${Boost_INCLUDE_DIR}
   ${SIM_CONNECT_INCLUDE_DIR}
   ${fsuipc_user_HEADERS}
   ${liboac-commons_HEADERS}
   )

add_library(WilcoExporter SHARED ${wilco-exporter_SOURCES})

target_link_libraries(WilcoExporter
   ${Boost_LIBRARIES}
   ${SIM_CONNECT_LIBRARY}
   fsuipc_user_internal
   liboac-commons
)
