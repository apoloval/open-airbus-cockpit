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

#pragma warning( disable : 4290 )

namespace oac {

struct Error : virtual std::exception, virtual boost::exception
{
   virtual const char* what() const throw ()
   {
      return boost::diagnostic_information_what(*this);
   }

   operator boost::exception_ptr() const {
      return boost::copy_exception(*this);
   }
};


struct LogicErrorBase : virtual Error {};
struct RuntimeErrorBase : virtual Error {};

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

#define DECL_LOGIC_ERROR(name) struct name : virtual LogicErrorBase {}
#define DECL_RUNTIME_ERROR(name) struct name : virtual RuntimeErrorBase {}
#define DECL_ERROR(name, parent) struct name : virtual parent {}

#define DECL_ERROR_INFO(name, type) \
   typedef boost::error_info<struct name##Tag, type> name

#define THROW_ERROR(error) BOOST_THROW_EXCEPTION(error)

namespace oac {

DECL_LOGIC_ERROR(ConnectionError);
DECL_LOGIC_ERROR(InvalidInputError);
DECL_LOGIC_ERROR(NotFoundError);
DECL_LOGIC_ERROR(NullPointerError);

DECL_RUNTIME_ERROR(IllegalStateError);
DECL_RUNTIME_ERROR(IOError);

DECL_ERROR_INFO(CodeInfo, int);
DECL_ERROR_INFO(FileNameInfo, std::wstring);
DECL_ERROR_INFO(FunctionNameInfo, std::string);
DECL_ERROR_INFO(IndexInfo, int);
DECL_ERROR_INFO(LowerBoundInfo, int);
DECL_ERROR_INFO(MessageInfo, std::string);
DECL_ERROR_INFO(NestedErrorInfo, boost::exception_ptr);
DECL_ERROR_INFO(UpperBoundInfo, int);

} // namespace oac

#endif
