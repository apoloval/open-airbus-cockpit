cmake_minimum_required(VERSION 2.8)

project(liboac-commons)

set(liboac-commons_HEADERS
   include/liboac/attempt.h
   include/liboac/buffer.h
   include/liboac/buffer/asio_handler.h
   include/liboac/buffer/double.h
   include/liboac/buffer/errors.h
   include/liboac/buffer/fixed.h
   include/liboac/buffer/fixed.inl
   include/liboac/buffer/functions.h
   include/liboac/buffer/linear.h
   include/liboac/buffer/linear.inl
   include/liboac/buffer/ring.h
   include/liboac/buffer/ring.inl
   include/liboac/buffer/shifted.h
   include/liboac/cockpit.h
   include/liboac/cockpit-fsuipc.h
   include/liboac/concurrency.h
   include/liboac/endian.h
   include/liboac/exception.h
   include/liboac/filesystem.h
   include/liboac/format.h
   include/liboac/fsuipc.h
   include/liboac/fsuipc/client.h
   include/liboac/fsuipc/errors.h
   include/liboac/fsuipc/local.h
   include/liboac/fsuipc/offset.h
   include/liboac/fsuipc/update_observer.h
   include/liboac/io.h
   include/liboac/logging.h
   include/liboac/network.h
   include/liboac/network/asio_utils.h
   include/liboac/network/async_client.h
   include/liboac/network/async_client.inl
   include/liboac/network/async_connection.h
   include/liboac/network/async_connection.inl
   include/liboac/network/async_server.h
   include/liboac/network/async_server.inl
   include/liboac/network/client.h
   include/liboac/network/client.inl
   include/liboac/network/connection.h
   include/liboac/network/connection.inl
   include/liboac/network/errors.h
   include/liboac/network/server.h
   include/liboac/network/server.inl
   include/liboac/network/types.h
   include/liboac/simconn.h
   include/liboac/stream.h
   include/liboac/stream/adapters.h
   include/liboac/stream/functions.h
   include/liboac/timing.h
   include/liboac/worker.h
)

set(liboac-commons_SOURCES
   src/cockpit.cpp
   src/cockpit-fsuipc.cpp
   src/filesystem.cpp
   src/fsuipc/client.cpp
   src/fsuipc/local.cpp
   src/logging.cpp
   src/simconn.cpp
   src/stream.cpp
   src/timing.cpp
)

set(liboac-commons_INCLUDE_DIR
   ${CMAKE_CURRENT_SOURCE_DIR}/include
   PARENT_SCOPE)

include_directories(
   ${CMAKE_CURRENT_SOURCE_DIR}/include
   ${fsuipc_user_INCLUDE_DIR}
)

add_library(liboac-commons STATIC 
   ${liboac-commons_SOURCES} 
   ${liboac-commons_HEADERS})

target_link_libraries(liboac-commons
   ${Boost_LIBRARIES}
   ${SIM_CONNECT_LIBRARY}
   fsuipc_user_internal
)

test_link_libraries(
      liboac-commons
      fsuipc_user_internal
      ${SIM_CONNECT_LIBRARY}
      ${Boost_LIBRARIES}
)

add_unit_test(attempt-test liboac)
add_unit_test(buffer-test liboac)
add_unit_test(buffer/asio_handler-test liboac)
add_unit_test(concurrency-test liboac)
add_unit_test(exception-test liboac)
add_unit_test(filesystem-test liboac)
add_unit_test(fsuipc-test liboac)
add_unit_test(stream-test liboac)
add_unit_test(timing-test liboac)

add_integration_test(simconn-itest liboac)
add_integration_test(logging-itest liboac)
add_integration_test(network-itest liboac)
add_integration_test(timing-itest liboac)
