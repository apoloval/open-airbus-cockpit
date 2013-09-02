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

#include "api.h"

namespace oac { namespace fv {

namespace {

template <typename T>
ptr<linear_buffer> make_buffer_of(const T& value)
{
   auto buff = new linear_buffer(sizeof(T));
   buffer::write_as(*buff, 0, value);
   return buff;
}

} // anonymous namespace

bool
variable_value::operator == (
      const variable_value& val) const
{
   if (val._type != _type)
      return false;
   switch (_type)
   {
      case variable_type::BOOLEAN: return val.as_bool() == as_bool();
      case variable_type::BYTE: return val.as_byte() == as_byte();
      case variable_type::WORD: return val.as_word() == as_word();
      case variable_type::DWORD: return val.as_dword() == as_dword();
      case variable_type::FLOAT: return val.as_float() == as_float();
      default:
         OAC_THROW_EXCEPTION(enum_out_of_range_error<variable_type>()
               .with_value(_type));
   }
}

variable_value
variable_value::from_bool(bool value)
{ return variable_value(variable_type::BOOLEAN, make_buffer_of<bool>(value)); }

variable_value
variable_value::from_byte(std::uint8_t value)
{ return variable_value(variable_type::BYTE, make_buffer_of(value)); }

variable_value
variable_value::from_word(std::uint16_t value)
{ return variable_value(variable_type::WORD, make_buffer_of(value)); }

variable_value
variable_value::from_dword(std::uint32_t value)
{ return variable_value(variable_type::DWORD, make_buffer_of(value)); }

variable_value
variable_value::from_float(float value)
{ return variable_value(variable_type::FLOAT, make_buffer_of(value)); }

bool
variable_value::as_bool() const
throw (invalid_type_error)
{
   check_type(variable_type::BOOLEAN);
   return buffer::read_as<bool>(*_buffer, 0);
}

std::uint8_t
variable_value::as_byte() const
throw (invalid_type_error)
{
   check_type(variable_type::BYTE);
   return buffer::read_as<std::uint8_t>(*_buffer, 0);
}

std::uint16_t
variable_value::as_word() const
throw (invalid_type_error)
{
   check_type(variable_type::WORD);
   return buffer::read_as<std::uint16_t>(*_buffer, 0);
}

std::uint32_t
variable_value::as_dword() const
throw (invalid_type_error)
{
   check_type(variable_type::DWORD);
   return buffer::read_as<std::uint32_t>(*_buffer, 0);
}

float
variable_value::as_float() const
throw (invalid_type_error)
{
   check_type(variable_type::FLOAT);
   return buffer::read_as<float>(*_buffer, 0);
}

std::string
variable_value::to_string() const
{
   switch (_type)
   {
      case variable_type::BOOLEAN: return as_bool() ? "true(bool)" : "false(bool)";
      case variable_type::BYTE: return str(boost::format("%d(byte)") % int(as_byte()));
      case variable_type::WORD: return str(boost::format("%d(word)") % as_word());
      case variable_type::DWORD: return str(boost::format("%d(dword)") % as_dword());
      case variable_type::FLOAT: return str(boost::format("%f(float)") % as_float());
      default:
         // never reached
         OAC_THROW_EXCEPTION(enum_out_of_range_error<variable_type>()
               .with_value(_type));
   }
}

void
variable_value::check_type(const variable_type& type) const
throw (invalid_type_error)
{
   if (_type != type)
      OAC_THROW_EXCEPTION(invalid_type_error()
            .with_expected_type(_type)
            .with_actual_type(type));
}


}} // namespace oac::fv
