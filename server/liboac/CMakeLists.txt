cmake_minimum_required(VERSION 2.8)

project(liboac)

# Set some variables that describe the build environment
set(liboac_INCLUDEDIR ${PROJECT_SOURCE_DIR}/include CACHE INTERNAL "")
set(liboac_SRCDIR ${PROJECT_SOURCE_DIR}/src)

# Set the sources and headers lists
aux_source_directory(${liboac_SRCDIR} liboac_SOURCES)

set(liboac_HEADERS
   ${liboac_INCLUDEDIR}/oacsp-utils.h
   ${liboac_INCLUDEDIR}/components.h
   ${liboac_INCLUDEDIR}/config.h
   ${liboac_INCLUDEDIR}/devices.h
   ${liboac_INCLUDEDIR}/events.h
   ${liboac_INCLUDEDIR}/exception.h
   ${liboac_INCLUDEDIR}/oacsp.h
   ${liboac_INCLUDEDIR}/serial.h
   ${liboac_INCLUDEDIR}/test.h
   ${liboac_INCLUDEDIR}/types.h
)

# Set the liboac target
add_library(liboac SHARED ${liboac_SOURCES} ${liboac_HEADERS})
set_target_properties(liboac PROPERTIES
   DEFINE_SYMBOL "LIBOAC_BUILD=1"
   COMPILE_FLAGS "${oac_cxx_flags}"
   LINK_FLAGS "${oac_ld_flags}"
   RUNTIME_OUTPUT_NAME "oac"
   ARCHIVE_OUTPUT_NAME "oac"
)

# Set include directories
include_directories(${liboac_INCLUDEDIR} ${KAREN_CORE_INCLUDE_DIR})

# Set liboac dependencies
target_link_libraries(liboac ${KAREN_CORE_LIBRARIES})
