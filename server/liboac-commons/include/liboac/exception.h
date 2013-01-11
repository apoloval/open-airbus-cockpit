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

#ifndef OAC_SERVER_EXCEPTION_H
#define OAC_SERVER_EXCEPTION_H

#include <exception>
#include <string>

#include <Boost/format.hpp>

#pragma warning( disable : 4290 )

#define DECL_STD_ERROR(classname, stdexcept) \
   class classname : public stdexcept { \
   public: \
      inline classname(const std::string& what) : stdexcept(what) \
      {} \
      \
      inline classname(const boost::format& fmt) : stdexcept(str(fmt)) \
      {} \
   };

#define DECL_LOGIC_ERROR(classname) \
   DECL_STD_ERROR(classname, std::logic_error)

#define DECL_RUNTIME_ERROR(classname) \
   DECL_STD_ERROR(classname, std::runtime_error)

namespace oac {

DECL_LOGIC_ERROR(InvalidInputException);
DECL_LOGIC_ERROR(NotFoundException);
DECL_LOGIC_ERROR(NullPointerException);

DECL_RUNTIME_ERROR(IllegalStateException);
DECL_RUNTIME_ERROR(IOException);

}; // namespace oac

#endif
