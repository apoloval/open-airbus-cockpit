cmake_minimum_required(VERSION 2.8)

project(oac-server)

# Prepare Qt modules and other stuff
# find_package(Qt4 REQUIRED)
# include(${QT_USE_FILE})
# add_definitions(${QT_DEFINITIONS})

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} 
   "${CMAKE_SOURCE_DIR}/../cmake/modules/")

set(Boost_USE_STATIC_LIBS ON)

include(Testing)
find_package(Boost COMPONENTS unit_test_framework REQUIRED)
find_package(SimConnect REQUIRED)
find_package(FSUIPC_Internal REQUIRED)

enable_testing()

include_directories(
    ${Boost_INCLUDE_DIR}
    ${SIM_CONNECT_INCLUDE_DIR}
    ${FSUIPC_INTERNAL_INCLUDE_DIR}
	${CMAKE_SOURCE_DIR}/liboac-commons/include
)

add_definitions(-DUNICODE)


# Add the subdirs
add_subdirectory(liboac-commons)
add_subdirectory(wilco-exporter)
