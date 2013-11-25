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
#  This CMake module may be used to find Mosquitto library. It sets the
#  following variables after success.
#
#   - MOSQUITTO_FOUND
#   - MOSQUITTO_INCLUDE_DIR
#   - MOSQUITTO_LIBRARY
#

include(FindPackageHandleStandardArgs)

set(mosquitto_default_path
   to_cmake_path(
      "C:\\Program Files (x86)\\mosquitto"
   )
)

find_path(MOSQUITTO_INCLUDE_DIR
   NAMES mosquitto.h
   PATHS ${mosquitto_default_path}
   PATH_SUFFIXES devel)

find_library(MOSQUITTO_LIBRARY
   NAMES mosquitto.lib
   PATHS ${mosquitto_default_path}
   PATH_SUFFIXES devel
)

find_package_handle_standard_args(Mosquitto DEFAULT_MSG
   MOSQUITTO_INCLUDE_DIR
   MOSQUITTO_LIBRARY)

mark_as_advanced(MOSQUITTO_LIBRARY MOSQUITTO_INCLUDE_DIR)
