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

#include "config.h"

namespace oac {

class LIBOAC_EXPORT Exception : public std::exception
{
public:
   
   inline Exception(const std::string& message) : _message(message) {}
   
   inline virtual ~Exception() throw() {}
   
   inline virtual const char* what() const throw()
   { return _message.c_str(); }

private:

   std::string _message;

};

#define DECL_EXCEPTION(type) \
   class type : public ::oac::Exception \
   {\
   public:\
      inline type(const std::string& message) : Exception(message) {} \
   }

DECL_EXCEPTION(IllegalStateException);
DECL_EXCEPTION(InvalidInputException);
DECL_EXCEPTION(IOException);
DECL_EXCEPTION(NotFoundException);
DECL_EXCEPTION(NullPointerException);

}; // namespace oac

#endif
