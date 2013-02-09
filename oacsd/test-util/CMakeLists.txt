cmake_minimum_required(VERSION 2.8)

project(oac-test-util)

# Set some variables that describe the build environment
set(testutil_INCLUDEDIR ${PROJECT_SOURCE_DIR}/include)
set(testutil_SRCDIR ${PROJECT_SOURCE_DIR}/src)
set(testutil_BUILDDIR ${PROJECT_SOURCE_DIR}/build)

# Set the sources and headers lists
aux_source_directory(${testutil_SRCDIR} testutil_SOURCES)

set(testutil_HEADERS
   ${testutil_INCLUDEDIR}/ctrl-fcu.h
   ${testutil_INCLUDEDIR}/ctrl-main.h
   ${testutil_INCLUDEDIR}/ui-fcu.h
   ${testutil_INCLUDEDIR}/ui-main.h
)

set(testutil_MOCS
   ${testutil_INCLUDEDIR}/ui-fcu.h
   ${testutil_INCLUDEDIR}/ui-main.h
)

qt4_wrap_cpp(testutil_MOC_OUTPUTS ${testutil_MOCS})

include_directories(
   ${testutil_INCLUDEDIR}
   ${KAREN_CORE_INCLUDE_DIR}
   ${liboac_INCLUDEDIR}
)

add_executable(testutil ${testutil_SOURCES} ${testutil_MOC_OUTPUTS})
set_target_properties(testutil PROPERTIES
   COMPILE_FLAGS "${flag_std_cxx11}"
   LINK_FLAGS "${TARGET_LINK_FLAGS}"
)
target_link_libraries(testutil
   ${QT_LIBRARIES}
   ${KAREN_CORE_LIBRARIES}
   liboac
)
