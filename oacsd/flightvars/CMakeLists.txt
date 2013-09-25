cmake_minimum_required(VERSION 2.8)

project(FlightVars)

set(libflightvars_HEADERS
   include/flightvars/api.h
   include/flightvars/client.h
   include/flightvars/client/connection_manager.h
   include/flightvars/client/errors.h
   include/flightvars/client/requests.h
   include/flightvars/client/subscription_db.h
   include/flightvars/protocol.h
   include/flightvars/proto/binary.h
   include/flightvars/proto/deserial.h
   include/flightvars/proto/errors.h
   include/flightvars/proto/messages.h
   include/flightvars/proto/serial.h
   include/flightvars/proto/types.h
   include/flightvars/subscription.h
   include/flightvars/var.h
)

set(libflightvars_SOURCES
   src/lib/api.cpp
   src/lib/client.cpp
   src/lib/client/connection_manager.cpp
   src/lib/client/subscription_db.cpp
   src/lib/subscription.cpp
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
   src/module/server.cpp
)

set(flightvars_INCLUDE_DIR
   ${CMAKE_CURRENT_SOURCE_DIR}/include
   PARENT_SCOPE)

include_directories(
   ${CMAKE_CURRENT_SOURCE_DIR}/include
   ${CMAKE_CURRENT_SOURCE_DIR}/src/module
   ${Boost_INCLUDE_DIR}
   ${SIM_CONNECT_INCLUDE_DIR}
   ${fsuipc_user_INCLUDE_DIR}
   ${liboac_INCLUDE_DIR}
   )

add_library(libflightvars STATIC ${libflightvars_SOURCES} ${libflightvars_HEADERS})
add_library(FlightVars SHARED ${flightvars_SOURCES} ${flightvars_HEADERS})
add_library(FlightVars_Static STATIC ${flightvars_SOURCES} ${flightvars_HEADERS})

target_link_libraries(FlightVars
   libflightvars
   ${Boost_LIBRARIES}
   ${SIM_CONNECT_LIBRARY}
   fsuipc_user_internal
   liboac
)

test_link_libraries(
   liboac
   libflightvars
   FlightVars_Static
   ${Boost_LIBRARIES}
   ${SIM_CONNECT_LIBRARY}
   fsuipc_user_internal
)

add_unit_test(client/requests-test flightvars_client)
add_unit_test(client/subscription_db-test flightvars_client)
add_unit_test(fsuipc-test flightvars)
add_unit_test(protocol-test flightvars)
add_unit_test(subscription-test flightvars)
add_unit_test(var-test flightvars)

add_integration_test(client-itest flightvars)
add_integration_test(server-itest flightvars)

