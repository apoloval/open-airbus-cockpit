cmake_minimum_required(VERSION 2.8)

project(FlightVars)

###
# Project files
#
include_directories(
   ${CMAKE_CURRENT_SOURCE_DIR}/include
   ${CMAKE_CURRENT_SOURCE_DIR}/src
   ${Boost_INCLUDE_DIR}
   ${MOSQUITTO_INCLUDE_DIR}
   ${SIM_CONNECT_INCLUDE_DIR}
   ${fsuipc_user_INCLUDE_DIR}
   ${liboac_INCLUDE_DIR}
)

set(flightvars_INCLUDE_DIR
   ${CMAKE_CURRENT_SOURCE_DIR}/include
   PARENT_SCOPE)

set(flightvars_SOURCES
   FlightVars.def
   src/dllmain.cpp
   src/main.cpp
   src/module.h
   src/conf/provider.cpp
   src/conf/settings.cpp
   src/mqtt/broker.h
   src/mqtt/broker_mosquitto.cpp
   src/mqtt/client.cpp
   src/mqtt/client_mosquitto.cpp
   src/mqtt/message.h
   src/mqtt/topic.h
   src/mqtt/qos.h
)

set(flightvars_UNIT_TESTS
   test/unit-test/conf/provider-test.h
   test/unit-test/mqtt/message-test.h
   test/unit-test/mqtt/topic-test.h
)

set(flightvars_INTEGRATION_TESTS
   test/integration-test/conf/provider-itest.h
   test/integration-test/mqtt/broker-itest.h
   test/integration-test/mqtt/client-itest.h
)

###
# FlightVars plugin DLL target
#
add_library(FlightVars SHARED ${flightvars_SOURCES} ${flightvars_HEADERS})

target_link_libraries(FlightVars
   ${Boost_LIBRARIES}
   ${MOSQUITTO_LIBRARY}
   ${SIM_CONNECT_LIBRARY}
   fsuipc_user_internal
   liboac
)

###
# FlightVars static library (for testing purposes)
#
add_library(FlightVars_Static STATIC ${flightvars_SOURCES} ${flightvars_HEADERS})

###
# Unit tests executable
#
add_executable(flightvars-unittest
   test/unit-test/main.cpp
   ${flightvars_UNIT_TESTS}
)

target_link_libraries(flightvars-unittest
   liboac
   FlightVars_Static
   ${Boost_LIBRARIES}
   ${MOSQUITTO_LIBRARY}
   ${SIM_CONNECT_LIBRARY}
   fsuipc_user_internal
)

add_test(NAME "flightvars unit tests" COMMAND flightvars-unittest)

###
# Integration tests executable
#
add_executable(flightvars-integrationtest
   test/integration-test/main.cpp
   ${flightvars_INTEGRATION_TESTS}
)

target_link_libraries(flightvars-integrationtest
   liboac
   FlightVars_Static
   ${Boost_LIBRARIES}
   ${MOSQUITTO_LIBRARY}
   ${SIM_CONNECT_LIBRARY}
   fsuipc_user_internal
)
