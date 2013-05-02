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
 * along with Open Airbus Cockpit. If not, see <http://www.gnu.org/licenses/>.
 */

#include <boost/algorithm/string.hpp>

#include "api.h"

namespace oac { namespace fv {

namespace {

template <typename T>
ptr<buffer> MakeBufferOf(const T& value)
{
   auto buffer = new fixed_buffer(sizeof(T));
   buffer->write_as<T>(0, value);
   return buffer;
}

} // anonymous namespace

tagged_element::tagged_element(const tag& tag) : _tag(tag)
{
   boost::algorithm::to_lower(_tag);
}

variable_value
variable_value::from_bool(bool value)
{ return variable_value(VAR_BOOLEAN, MakeBufferOf<bool>(value)); }

variable_value
variable_value::from_byte(BYTE value)
{ return variable_value(VAR_BYTE, MakeBufferOf<BYTE>(value)); }

variable_value
variable_value::from_word(WORD value)
{ return variable_value(VAR_WORD, MakeBufferOf<WORD>(value)); }

variable_value
variable_value::from_dword(DWORD value)
{ return variable_value(VAR_DWORD, MakeBufferOf<DWORD>(value)); }

variable_value
variable_value::from_float(float value)
{ return variable_value(VAR_FLOAT, MakeBufferOf<float>(value)); }

bool
variable_value::as_bool() const
throw (invalid_type_error)
{
   checkType(VAR_BOOLEAN);
   return _buffer->read_as<bool>(0);
}

BYTE
variable_value::as_byte() const
throw (invalid_type_error)
{
   checkType(VAR_BYTE);
   return _buffer->read_as<BYTE>(0);
}

WORD
variable_value::as_word() const
throw (invalid_type_error)
{
   checkType(VAR_WORD);
   return _buffer->read_as<WORD>(0);
}

DWORD
variable_value::as_dword() const
throw (invalid_type_error)
{
   checkType(VAR_DWORD);
   return _buffer->read_as<DWORD>(0);
}

float
variable_value::as_float() const
throw (invalid_type_error)
{
   checkType(VAR_FLOAT);
   return _buffer->read_as<float>(0);
}

void
variable_value::checkType(const variable_type& type) const
throw (invalid_type_error)
{
   if (_type != type)
      BOOST_THROW_EXCEPTION(invalid_type_error() <<
            expected_type_info(type) << actual_type_info(_type));
}


}} // namespace oac::fv
