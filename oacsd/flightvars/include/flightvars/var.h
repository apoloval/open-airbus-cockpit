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

#ifndef OAC_FV_VAR_H
#define OAC_FV_VAR_H

#include <boost/algorithm/string.hpp>
#include <liboac/buffer.h>
#include <liboac/exception.h>
#include <liboac/format.h>

namespace oac { namespace fv {

typedef std::string variable_group;

typedef std::string variable_name;

struct variable_id
{

   OAC_DECL_EXCEPTION_WITH_PARAMS(parse_error, oac::exception,
      ("not a valid format for a variable ID in '%s'", token),
      (token, std::string)
   );

   static variable_id
   parse(const std::string& var)
   {
      std::vector<std::string> tokens;
      boost::iter_split(tokens, var, boost::algorithm::first_finder("->"));
      if (tokens.size() != 2 || tokens[0].empty() || tokens[1].empty())
         OAC_THROW_EXCEPTION(parse_error(var));
      return variable_id(tokens[0], tokens[1]);
   }

   const variable_group group;
   const variable_name name;

   variable_id(
         const variable_group& group,
         const variable_name& name)
      : group(boost::algorithm::to_lower_copy(group)),
        name(boost::algorithm::to_lower_copy(name))
   {
   }

   std::string to_string() const
   { return format("%s->%s", group, name); }

   bool operator == (const variable_id& var_id) const
   { return group == var_id.group && name == var_id.name; }

   bool operator < (const variable_id& var_id) const
   {
      return (group != var_id.group) ?
            group < var_id.group :
            name < var_id.name;
   }
};

inline std::ostream& operator << (
      std::ostream& s,
      const variable_id& var_id)
{ return s << var_id.to_string(); }



/**
 * A hash function for a variable ID.
 */
struct variable_id_hash
{
   std::hash<std::string> str_hash;

   std::size_t operator()(const variable_id& var_id) const
   {
      return str_hash(var_id.group) + str_hash(var_id.name);
   }
};

enum class variable_type
{
   BOOLEAN,
   BYTE,
   WORD,
   DWORD,
   FLOAT
};

/**
 * Convert a variable type into string.
 */
inline std::string
var_type_to_string(
      variable_type var_type)
{
   switch (var_type)
   {
      case variable_type::BOOLEAN : return "bool";
      case variable_type::BYTE : return "byte";
      case variable_type::WORD : return "word";
      case variable_type::DWORD : return "dword";
      case variable_type::FLOAT : return "float";
      default:
         // never reached
         OAC_THROW_EXCEPTION(enum_out_of_range_error<variable_type>(var_type));
   }
}

/**
 * Output stream operator for variable_type.
 */
inline std::ostream& operator <<(
      std::ostream& s,
      const variable_type& var_type)
{ return s << var_type_to_string(var_type); }



struct variable_value
{
   OAC_DECL_EXCEPTION_WITH_PARAMS(invalid_type_error, oac::exception,
      (
         "invalid variable type %s (expected %s)",
         var_type_to_string(actual_type),
         var_type_to_string(expected_type)
      ),
      (expected_type, variable_type),
      (actual_type, variable_type));

   bool operator == (const variable_value& val) const;

   static variable_value from_bool(bool value);
   static variable_value from_byte(std::uint8_t value);
   static variable_value from_word(std::uint16_t value);
   static variable_value from_dword(std::uint32_t value);
   static variable_value from_float(float value);

   inline variable_type get_type() const { return _type; }

   bool as_bool() const throw (invalid_type_error);
   std::uint8_t as_byte() const throw (invalid_type_error);
   std::uint16_t as_word() const throw (invalid_type_error);
   std::uint32_t as_dword() const throw (invalid_type_error);
   float as_float() const throw (invalid_type_error);

   std::string to_string() const;

private:

   variable_type _type;
   buffer::linear_buffer_ptr _buffer;

   inline variable_value(
         const variable_type& type,
         const buffer::linear_buffer_ptr& buffer)
      : _type(type), _buffer(buffer) {}

   void check_type(
         const variable_type& get_type) const
   throw (invalid_type_error);
};

/**
 * Output stream operator for variable_value.
 */
inline std::ostream& operator <<(
      std::ostream& s,
      const variable_value& var_value)
{ return s << var_value.to_string(); }

}} // namespace oac::fv

#endif
