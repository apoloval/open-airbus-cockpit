#
#  This file is part of Open Airbus Cockpit
#  Copyright (C) 2012, 2013 Alvaro Polo
#
#  Open Airbus Cockpit is free software: you can redistribute it and/or
#  modify it under the terms of the GNU General Public License as published
#  by the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  Open Airbus Cockpit is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with Open Airbus Cockpit.  If not, see <http://www.gnu.org/licenses/>.
#
#  ------------
#
#  This CMake module may be used to configure Visual C++ runtime in order
#  to indicate it have to be dynamically or statically linked.
#
#  It works by defining a value for MSVC_RUNTIME_LINK variable, which may
#  be one of:
#
#    'static'  -> MSVC runtime will be statically linked
#    'dynamic' -> MSVC runtime will be dinamically linked
#
#  Any other value will stop with the corresponding error. If MSVC_RUNTIME_LINK
#  is undefined, it will be defaulted to 'dynamic'.
#

macro(configure_msvc_runtime)
   if(MSVC)
      if("${MSVC_RUNTIME_LINK}" STREQUAL "")
         set(MSVC_RUNTIME_LINK "dynamic")
      endif()

      # Set compiler options.
      set(variables
         CMAKE_C_FLAGS_DEBUG
         CMAKE_C_FLAGS_MINSIZEREL
         CMAKE_C_FLAGS_RELEASE
         CMAKE_C_FLAGS_RELWITHDEBINFO
         CMAKE_CXX_FLAGS_DEBUG
         CMAKE_CXX_FLAGS_MINSIZEREL
         CMAKE_CXX_FLAGS_RELEASE
         CMAKE_CXX_FLAGS_RELWITHDEBINFO
      )
      if(${MSVC_RUNTIME_LINK} STREQUAL "static")
         message(STATUS
            "MSVC -> forcing use of statically-linked runtime.")
         foreach(variable ${variables})
            if(${variable} MATCHES "/MD")
               string(REGEX REPLACE "/MD" "/MT" ${variable} "${${variable}}")
            endif()
         endforeach()
      elseif(${MSVC_RUNTIME_LINK} STREQUAL "dynamic")
         message(STATUS
            "MSVC -> forcing use of dynamically-linked runtime.")
         foreach(variable ${variables})
            if(${variable} MATCHES "/MT")
               string(REGEX REPLACE "/MT" "/MD" ${variable} "${${variable}}")
            endif()
         endforeach()
      else()
         message(SEND_ERROR
            "invalid value ${MSVC_RUNTIME_LINK} for MSVC_RUNTIME_LINK variable")
      endif()
   endif()
endmacro()
