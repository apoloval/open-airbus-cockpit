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
#  This CMake module may be used to find SimConnect library. It sets the
#  following variables after success.
#
#   - SIM_CONNECT_FOUND
#   - SIM_CONNECT_INCLUDE_DIR
#   - SIM_CONNECT_LIBRARY
#  

include(FindPackageHandleStandardArgs)

set(sim_connect_default_path 
	to_cmake_path(
		"C:\\Program Files (x86)\\Microsoft Games\\Microsoft Flight Simulator X SDK\\SDK\\Core Utilities Kit\\SimConnect SDK"
	)
)

find_path(SIM_CONNECT_INCLUDE_DIR
	NAMES SimConnect.h
	PATHS ${sim_connect_default_path}
	PATH_SUFFIXES inc)

find_library(SIM_CONNECT_LIBRARY 
	NAMES SimConnect.lib
	PATHS ${sim_connect_default_path}
	PATH_SUFFIXES lib
)

find_package_handle_standard_args(SimConnect DEFAULT_MSG 
	SIM_CONNECT_INCLUDE_DIR
	SIM_CONNECT_LIBRARY)

mark_as_advanced(SIM_CONNECT_LIBRARY SIM_CONNECT_INCLUDE_DIR)
