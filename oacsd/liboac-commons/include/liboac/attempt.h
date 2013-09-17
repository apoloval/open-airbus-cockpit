/*
 * This file is part of Open Airbus Cockpit
 * Copyright (C) 2012, 2013 Alvaro Polo
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

#ifndef OAC_ATTEMPT_H
#define OAC_ATTEMPT_H

#include "exception.h"

namespace oac {

/**
 * An object representing an attempt to produce a value. This object
 * encapsulates either a value of type T or an exception. It may be used to
 * represent an attempt to compute a result. If such computation success,
 * the get_value() member function returns its result. Otherwise it throws
 * an exception. The successful value or error are passed to the class
 * constructor.
 */
template <typename T>
class attempt
{
public:

   /**
    * Create a new success attempt from given value.
    *
    * @param value The successful value
    */
   attempt(const T& value) : _value(std::make_unique<T>(value)) {}

   /**
    * Create a failed attempt from given exception.
    *
    * @param error The reason to fail
    */
   template <typename Exception>
   attempt(const Exception& error) : _error(std::make_exception_ptr(error)) {}

   /**
    * Copy constructor.
    */
   attempt(const attempt& a)
      : _value(a._value ? std::make_unique<T>(*a._value) : nullptr),
        _error(a._error)
   {}

   /**
    * Assign operator.
    */
   attempt& operator = (const attempt& a)
   {
      _value = a._value ? std::make_unique<T>(*a._value) : nullptr;
      _error = a._error;
      return *this;
   }

   /**
    * Get the value of the attempt. If the attempt is successful, it returns
    * the value provided in the constructor. Otherwise, the exception that was
    * injected in construction is thrown.
    *
    * @return The successful value
    */
   const T& get_value() const
   {
      if (!_value)
         std::rethrow_exception(_error);
      return *_value;
   }

private:

   typedef std::unique_ptr<T> value_uptr;

   value_uptr _value;
   std::exception_ptr _error;

};

/**
 * Make a successful attempt.
 */
template <typename T>
attempt<T> make_success(const T& t)
{
   return attempt<T>(t);
}

/**
 * Make a failed attempt.
 */
template <typename T, typename Exception>
attempt<T> make_failure(const Exception& error)
{
   return attempt<T>(error);
}

} // namespace oac

#endif
