#
#  This file is part of Open Airbus Cockpit
#  Copyright (C) 2012 Alvaro Polo
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
#  This CMake module provide some macros and functions package using
#  NSIS.
#

find_program(makensis_BIN
   NAMES makensis.exe
   PATHS "$ENV{ProgramFiles(x86)}/NSIS")

add_custom_target(nsis)

function(add_installer target_name script_file)
   set(ARGSM DEPENDS)
   cmake_parse_arguments(
      add_installer "${ARGS0}" "${ARGS1}" "${ARGSM}" ${ARGN})

   set(add_installer_OUTPUT "${CMAKE_BINARY_DIR}/${target_name}.exe")
   file(TO_NATIVE_PATH ${CMAKE_SOURCE_DIR} add_installer_SOURCE_DIR)
   file(TO_NATIVE_PATH ${CMAKE_BINARY_DIR} add_installer_BINARY_DIR)

   add_custom_command(
      OUTPUT ${add_installer_OUTPUT}
      COMMAND ${makensis_BIN}
         /DCMAKE_NSIS_OUTPUT=${add_installer_OUTPUT}
         /DCMAKE_SOURCE_DIR=${add_installer_SOURCE_DIR}
         /DCMAKE_BINARY_DIR=${add_installer_BINARY_DIR}
         ${script_file}
      DEPENDS ${script_file} ${add_installer_DEPENDS}
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

   add_custom_target(
      ${target_name} DEPENDS ${add_installer_OUTPUT})

   add_dependencies(nsis ${target_name})

endfunction(add_installer)
