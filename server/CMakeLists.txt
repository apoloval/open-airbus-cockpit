cmake_minimum_required(VERSION 2.8)

project(oac)

# Prepare Qt modules and other stuff
find_package(Qt4 REQUIRED)
include(${QT_USE_FILE})
add_definitions(${QT_DEFINITIONS})

# Set some global variables
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
   set(oac_cxx_flags "-std=c++11")
   set(oac_ld_flags "-static-libgcc -static-libstdc++")
endif()

# Determine the location of Karen-Core
if (KAREN_CORE_DLL)
   find_library(KAREN_CORE_LIBRARIES KarenCore ${KAREN_CORE_DLL})
else()
   message(FATAL_ERROR "cannot find KarenCore library" 
         "please set KAREN_CORE_DLL variable to either DLL file or folder")
endif()
if (KAREN_CORE_INCLUDE_DIR)
else()
   message(FATAL_ERROR "cannot find KarenCore headers"
         "please set KAREN_CORE_INCLUDE_DIR variable to the include folder")
endif()

# Add the subdirs
add_subdirectory(liboac)
add_subdirectory(test-util)
