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
#  This CMake module may be used to find FSUIPC internal library. It sets the
#  following variables after success.
#
#   - FSUIPC_INTERNAL_FOUND
#   - FSUIPC_INTERNAL_INCLUDE_DIR
#   - FSUIPC_INTERNAL_LIBRARY
#  

include(FindPackageHandleStandardArgs)

find_path(FSUIPC_INTERNAL_INCLUDE_DIR
	NAMES FSUIPC_User.h
	PATHS ENV FSUIPC_INTERNAL_HOME ENV FSUIPC_INTERNAL_DIR
)

find_library(FSUIPC_INTERNAL_LIBRARY 
	NAMES ModuleUser.lib FSUIPC_User.lib Module_User.lib
	PATHS ENV FSUIPC_INTERNAL_HOME ENV FSUIPC_INTERNAL_DIR
)

find_package_handle_standard_args(FSUIPC_Internal
	"cannot find FSUIPC Internal library; please set FSUIPC_INTERNAL_DIR environment variable to the folder that contains the header and the library"
	FSUIPC_INTERNAL_INCLUDE_DIR
	FSUIPC_INTERNAL_LIBRARY)

mark_as_advanced(FSUIPC_INTERNAL_INCLUDE_DIR FSUIPC_INTERNAL_LIBRARY)
