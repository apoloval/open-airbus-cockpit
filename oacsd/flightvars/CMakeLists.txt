cmake_minimum_required(VERSION 2.8)

project(FlightVars)

set(libflightvars_HEADERS
   include/flightvars/api.h
   include/flightvars/protocol.h
   include/flightvars/protocol.inl
)

set(libflightvars_SOURCES
   src/module/api.cpp
)

set(flightvars_HEADERS
   include/flightvars/core.h
)

set(flightvars_SOURCES
   FlightVars.def
   src/module/core.cpp
   src/module/dllmain.cpp
   src/module/fsuipc.cpp
   src/module/main.cpp
   src/module/protocol.cpp
   src/module/server.cpp
   src/module/subscription.cpp
)

include_directories(
   ${CMAKE_CURRENT_SOURCE_DIR}/include/flightvars
   ${CMAKE_CURRENT_SOURCE_DIR}/src/module
   ${Boost_INCLUDE_DIR}
   ${SIM_CONNECT_INCLUDE_DIR}
   ${FSUIPC_INTERNAL_INCLUDE_DIR}
   ${liboac-commons_HEADERS}
   )

add_library(libflightvars STATIC ${libflightvars_SOURCES} ${libflightvars_HEADERS})
add_library(FlightVars SHARED ${flightvars_SOURCES} ${flightvars_HEADERS})
add_library(FlightVars_Static STATIC ${flightvars_SOURCES} ${flightvars_HEADERS})

target_link_libraries(FlightVars
   libflightvars
   ${Boost_LIBRARIES}
   ${SIM_CONNECT_LIBRARY}
   ${FSUIPC_INTERNAL_LIBRARY}
   liboac-commons
)

test_link_libraries(
   liboac-commons
   libflightvars
   FlightVars_Static
   ${Boost_LIBRARIES}
   ${SIM_CONNECT_LIBRARY}
   ${FSUIPC_INTERNAL_LIBRARY}
)

add_unit_test(api-test)
add_unit_test(protocol-test)

add_integration_test(fsuipc-itest)
add_integration_test(server-itest)

