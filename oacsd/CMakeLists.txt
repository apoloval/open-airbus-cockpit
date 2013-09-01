cmake_minimum_required(VERSION 2.8)

project(oacsd)

set(OACSD_VERSION_MAJOR 0)
set(OACSD_VERSION_MINOR 2)
set(OACSD_VERSION_PATH beta-1)
set(OACSD_VERSION
   "${OACSD_VERSION_MAJOR}.${OACSD_VERSION_MINOR}.${OACSD_VERSION_PATH}")

# Prepare Qt modules and other stuff
# find_package(Qt4 REQUIRED)
# include(${QT_USE_FILE})
# add_definitions(${QT_DEFINITIONS})

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules/" ${CMAKE_MODULE_PATH})

set(Boost_USE_STATIC_LIBS ON)

include(Testing)
find_package(Boost COMPONENTS
   chrono
   date_time
   filesystem
   regex
   system
   thread
   unit_test_framework
REQUIRED)
find_package(SimConnect REQUIRED)

enable_testing()

include_directories(
    ${Boost_INCLUDE_DIR}
    ${SIM_CONNECT_INCLUDE_DIR}
    ${FSUIPC_INTERNAL_INCLUDE_DIR}
	${CMAKE_SOURCE_DIR}/liboac-commons/include
)

add_definitions(
   /FS
   -DUNICODE
   -D_SCL_SECURE_NO_WARNINGS
   -D_VARIADIC_MAX=6
   -DNOMINMAX # Required to avoid clashing with std::min and std::max
)

set(CPACK_GENERATOR "NSIS")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "OACSD")
set(CPACK_PACKAGE_VERSION "${OACSD_VERSION}")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/COPYING")

set(CPACK_NSIS_COMPONENT_INSTALL ON)
set(CPACK_NSIS_PACKAGE_NAME "Open Airbus Cockpit Software Distribution")
set(CPACK_NSIS_DISPLAY_NAME "Open Airbus Cockpit Software Distribution")
set(CPACK_NSIS_CONTACT "apoloval@gmail.com")
set(CPACK_NSIS_MENU_LINKS
   "http://open-airbus-cockpit.blogspot.com.es/" "Open Airbus Cockpit Blog"
   "https://github.com/apoloval/open-airbus-cockpit" "Project in GitHub"
)

# Add the subdirs
add_subdirectory(fsuipc)
add_subdirectory(liboac-commons)
add_subdirectory(wilco-exporter)
add_subdirectory(flightvars)

# Packaging with NSIS
include(NSIS)
add_installer("oacsd-${OACSD_VERSION}" ${CMAKE_SOURCE_DIR}/NSIS/oacsd.nsh
   DEPENDS WilcoExporter
   VERSION ${OACSD_VERSION}
)

