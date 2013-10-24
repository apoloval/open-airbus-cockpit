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
#  This CMake module provide some macros and functions to support testing
#

#
# This macro may be used to indicate what are the link libraries for the
# tests defined in your CMake file. 
macro(test_link_libraries)
   set(test_libraries ${ARGV})
endmacro(test_link_libraries)

#
# Internal function. Do not use.
function(add_simple_test target_name_prefix testname is_unit)
   set(test_source "test/${testname}.cpp")
   get_filename_component(testname ${testname} NAME)
   if (target_name_prefix)
      set(target_name "${target_name_prefix}_${testname}")
   else()
      set(target_name ${testname})
   endif()

   add_executable(${target_name} ${test_source})
   target_link_libraries(${target_name} ${test_libraries})
   if (is_unit)
      add_test(NAME ${target_name} COMMAND ${target_name})
   endif()
endfunction(add_simple_test)

#
# Add a new integration test. This generates a executable target but doesn't
# include it as a test target
function(add_integration_test testname)
   set(target_name_prefix ${ARGV1})
   add_simple_test("${target_name_prefix}" ${testname} FALSE)
endfunction(add_integration_test)

#
# Add a new unit test. This generates a executable target and adds it as test
# target.
function(add_unit_test testname)
   set(target_name_prefix ${ARGV1})
   add_simple_test("${target_name_prefix}" ${testname} TRUE)
endfunction(add_unit_test)
