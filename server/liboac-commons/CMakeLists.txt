cmake_minimum_required(VERSION 2.8)

project(liboac-commons)

set(CMAKE_MODULE_PATH 
   ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/../../cmake/modules/")

set(Boost_USE_STATIC_LIBS ON)

find_package(Boost COMPONENTS unit_test_framework REQUIRED)
find_package(SimConnect REQUIRED)
find_package(FSUIPC_Internal REQUIRED)

set(liboac-commons_HEADERS
   include/buffer.h
   include/cockpit.h
   include/cockpit-fsuipc.h
   include/exception.h
   include/fsuipc.h
   include/logging.h
   include/pointer.h
   include/simconn.h
)

set(liboac-commons_SOURCES
   src/buffer.cpp
   src/cockpit.cpp
   src/cockpit-fsuipc.cpp
   src/fsuipc.cpp
   src/logging.cpp
   src/simconn.cpp
)

add_definitions(-DUNICODE)

include_directories(
   include
   ${Boost_INCLUDE_DIR}
   ${SIM_CONNECT_INCLUDE_DIR}
   ${FSUIPC_INTERNAL_INCLUDE_DIR}
   )

link_directories()

add_library(liboac-commons STATIC 
   ${liboac-commons_SOURCES} 
   ${liboac-commons_HEADERS})

set_target_properties(liboac-commons PROPERTIES
   DEFINE_SYMBOL "LIBOAC_COMMONS_BUILD=1"
   COMPILE_FLAGS ""
   LINK_FLAGS ""
)
target_link_libraries(liboac-commons
   ${SIM_CONNECT_LIBRARY}
   ${FSUIPC_INTERNAL_LIBRARY}
)

add_executable(simconn_itest test/simconn-itest.cpp)

target_link_libraries(simconn_itest
   liboac-commons
   ${SIM_CONNECT_LIBRARY}
   ${FSUIPC_INTERNAL_LIBRARY}
   ${Boost_LIBRARIES}
)

add_executable(RequestData test/RequestData.cpp)

target_link_libraries(RequestData
   liboac-commons
   ${SIM_CONNECT_LIBRARY}
   ${FSUIPC_INTERNAL_LIBRARY}
   ${Boost_LIBRARIES}
)
