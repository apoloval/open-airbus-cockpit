cmake_minimum_required(VERSION 2.8)

project(liboac)

###
# Project files
#
include_directories(
   ${CMAKE_CURRENT_SOURCE_DIR}/include
   ${fsuipc_user_INCLUDE_DIR}
)

set(liboac_INCLUDE_DIR
   ${CMAKE_CURRENT_SOURCE_DIR}/include
   PARENT_SCOPE)

set(liboac_HEADERS
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
   include/liboac/thread/channel.h
   include/liboac/thread/monitor.h
   include/liboac/thread/task.h
   include/liboac/timing.h
   include/liboac/util/attempt.h
   include/liboac/util/enum.h
   include/liboac/util/win.h
   include/liboac/worker.h
)

set(liboac_SOURCES
   src/cockpit.cpp
   src/cockpit-fsuipc.cpp
   src/filesystem.cpp
   src/fsuipc/client.cpp
   src/fsuipc/local.cpp
   src/logging.cpp
   src/simconn.cpp
   src/timing.cpp
)

set(liboac_UNIT_TESTS
   test/unit-test/buffer-test.h
   test/unit-test/buffer/asio_handler-test.h
   test/unit-test/concurrency-test.h
   test/unit-test/exception-test.h
   test/unit-test/filesystem-test.h
   test/unit-test/fsuipc-test.h
   test/unit-test/stream-test.h
   test/unit-test/thread/channel-test.h
   test/unit-test/thread/monitor-test.h
   test/unit-test/thread/task-test.h
   test/unit-test/timing-test.h
   test/unit-test/util/attempt-test.h
)

set(liboac_INTEGRATION_TESTS
   test/integration-test/logging-itest.h
   test/integration-test/simconn-itest.h
   test/integration-test/timing-itest.h
   test/integration-test/network/async_client-itest.h
   test/integration-test/network/async_server-itest.h
   test/integration-test/network/client-itest.h
   test/integration-test/network/server-itest.h
)

###
# liboac static library
#
add_library(liboac STATIC
   ${liboac_SOURCES}
   ${liboac_HEADERS})

target_link_libraries(liboac
   ${Boost_LIBRARIES}
   ${SIM_CONNECT_LIBRARY}
   fsuipc_user_internal
)

###
# Unit tests executable
#
add_executable(liboac-unittest test/unit-test/main.cpp ${liboac_UNIT_TESTS})

target_link_libraries(liboac-unittest
      liboac
      fsuipc_user_internal
      ${SIM_CONNECT_LIBRARY}
      ${Boost_LIBRARIES}
)
add_test(NAME "liboac unit tests" COMMAND liboac-unittest)

###
# Integration tests executable
#

add_executable(liboac-integrationtest
      test/integration-test/main.cpp
      ${liboac_INTEGRATION_TESTS}
)

target_link_libraries(liboac-integrationtest
      liboac
      fsuipc_user_internal
      ${SIM_CONNECT_LIBRARY}
      ${Boost_LIBRARIES}
)
