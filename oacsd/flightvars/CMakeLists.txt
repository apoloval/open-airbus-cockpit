cmake_minimum_required(VERSION 2.8)

project(FlightVars)

set(flightvars_SOURCES
   FlightVars.def
   src/mqtt/broker.cpp
   src/dllmain.cpp
   src/main.cpp
)

set(flightvars_INCLUDE_DIR
   ${CMAKE_CURRENT_SOURCE_DIR}/include
   PARENT_SCOPE)

include_directories(
   ${CMAKE_CURRENT_SOURCE_DIR}/include
   ${CMAKE_CURRENT_SOURCE_DIR}/src
   ${Boost_INCLUDE_DIR}
   ${SIM_CONNECT_INCLUDE_DIR}
   ${fsuipc_user_INCLUDE_DIR}
   ${liboac_INCLUDE_DIR}
   )

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
   FlightVars_Static
   ${Boost_LIBRARIES}
   ${SIM_CONNECT_LIBRARY}
   fsuipc_user_internal
)

add_integration_test(mqtt/broker-itest fv)
