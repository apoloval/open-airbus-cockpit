cmake_minimum_required(VERSION 2.8)

project(liboac-commons)

set(liboac-commons_HEADERS
   include/liboac/buffer.h
   include/liboac/cockpit.h
   include/liboac/cockpit-fsuipc.h
   include/liboac/exception.h
   include/liboac/fsuipc.h
   include/liboac/lang-utils.h
   include/liboac/logging.h
   include/liboac/simconn.h
)

set(liboac-commons_SOURCES
   src/buffer.cpp
   src/cockpit.cpp
   src/cockpit-fsuipc.cpp
   src/fsuipc.cpp
   src/logging.cpp
   src/simconn.cpp
)

include_directories(
   ${CMAKE_CURRENT_SOURCE_DIR}/include/liboac
)

add_library(liboac-commons STATIC 
   ${liboac-commons_SOURCES} 
   ${liboac-commons_HEADERS})

test_link_libraries(
      liboac-commons
      ${SIM_CONNECT_LIBRARY}
      ${FSUIPC_INTERNAL_LIBRARY}
      ${Boost_LIBRARIES}
)

add_unit_test(lang-utils-test)

add_integration_test(simconn-itest)
