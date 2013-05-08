/*
 * This file is part of Open Airbus Cockpit
 * Copyright (C) 2012 Alvaro Polo
 *
 * Open Airbus Cockpit is free software: you can redistribute it and/or 
 * modify it under the terms of the GNU General Public License as published 
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * Open Airbus Cockpit is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Open Airbus Cockpit.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef OAC_EXCEPTION_H
#define OAC_EXCEPTION_H

#include <exception>
#include <string>
#include <utility>

#include <boost/format.hpp>
#include <boost/exception/all.hpp>
#include <boost/system/system_error.hpp>

#pragma warning( disable : 4290 )

namespace oac {

struct error : virtual std::exception, virtual boost::exception
{
   virtual const char* what() const throw ()
   {
      return boost::diagnostic_information_what(*this);
   }

   operator boost::exception_ptr() const {
      return boost::copy_exception(*this);
   }
};


struct logic_error_base : virtual error {};
struct runtime_error_base : virtual error {};

template <typename ErrorInfo, typename E>
const typename ErrorInfo::error_info::value_type* GetErrorInfo(const E& e)
{ return boost::get_error_info<ErrorInfo>(e); }

} // namespace oac

#define DECL_STD_ERROR(classname, stdexcept) \
   class classname : public stdexcept { \
   public: \
      inline classname(const std::string& what) : stdexcept(what) \
      {} \
      \
      inline classname(const boost::format& fmt) : stdexcept(str(fmt)) \
      {} \
   };

#define OAC_DECL_LOGIC_ERROR(name) struct name : virtual logic_error_base {}
#define OAC_DECL_RUNTIME_ERROR(name) struct name : virtual runtime_error_base {}
#define OAC_DECL_ERROR(name, parent) struct name : virtual parent {}

#define OAC_DECL_ERROR_INFO(name, type) \
   typedef boost::error_info<struct name##Tag, type> name

namespace oac {

OAC_DECL_LOGIC_ERROR(connection_error);
OAC_DECL_LOGIC_ERROR(invalid_input_error);
OAC_DECL_LOGIC_ERROR(not_found_error);
OAC_DECL_LOGIC_ERROR(null_pointer_error);

OAC_DECL_RUNTIME_ERROR(illegal_state_error);
OAC_DECL_RUNTIME_ERROR(io_error);

OAC_DECL_ERROR_INFO(boost_system_error_info, boost::system::system_error);
OAC_DECL_ERROR_INFO(code_info, int);
OAC_DECL_ERROR_INFO(file_name_info, std::wstring);
OAC_DECL_ERROR_INFO(function_name_info, std::string);
OAC_DECL_ERROR_INFO(index_info, int);
OAC_DECL_ERROR_INFO(lower_bound_info, int);
OAC_DECL_ERROR_INFO(message_info, std::string);
OAC_DECL_ERROR_INFO(nested_error_info, boost::exception_ptr);
OAC_DECL_ERROR_INFO(upper_bound_info, int);

} // namespace oac

#endif
