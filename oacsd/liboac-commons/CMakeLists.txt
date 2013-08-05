cmake_minimum_required(VERSION 2.8)

project(liboac-commons)

set(liboac-commons_HEADERS
   include/liboac/buffer.h
   include/liboac/buffer.inl
   include/liboac/cockpit.h
   include/liboac/cockpit-fsuipc.h
   include/liboac/concurrency.h
   include/liboac/endian.h
   include/liboac/exception.h
   include/liboac/filesystem.h
   include/liboac/fsuipc.h
   include/liboac/lang-utils.h
   include/liboac/logging.h
   include/liboac/network.h
   include/liboac/network.inl
   include/liboac/simconn.h
   include/liboac/stream.h
   include/liboac/timing.h
   include/liboac/worker.h
)

set(liboac-commons_SOURCES
   src/cockpit.cpp
   src/cockpit-fsuipc.cpp
   src/filesystem.cpp
   src/fsuipc.cpp
   src/logging.cpp
   src/simconn.cpp
   src/stream.cpp
)

include_directories(
   ${CMAKE_CURRENT_SOURCE_DIR}/include/liboac
)

add_library(liboac-commons STATIC 
   ${liboac-commons_SOURCES} 
   ${liboac-commons_HEADERS})

test_link_libraries(
      liboac-commons
      fsuipc_user_internal
      ${SIM_CONNECT_LIBRARY}
      ${Boost_LIBRARIES}
)

add_unit_test(buffer-test liboac)
add_unit_test(concurrency-test liboac)
add_unit_test(filesystem-test liboac)
add_unit_test(fsuipc-test liboac)
add_unit_test(lang-utils-test liboac)
add_unit_test(network-test liboac)
add_unit_test(stream-test liboac)
add_unit_test(timing-test liboac)

add_integration_test(simconn-itest liboac)
add_integration_test(logging-itest liboac)
