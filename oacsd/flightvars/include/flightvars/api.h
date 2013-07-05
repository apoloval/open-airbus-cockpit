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

#ifndef OAC_FV_API_H
#define OAC_FV_API_H

#include <functional>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <boost/algorithm/string.hpp>
#include <liboac/buffer.h>
#include <liboac/exception.h>

namespace oac { namespace fv {

/**
 * A class to represent tagged elements such as var groups, var names, etc.
 * tags are case insensitive. The tag passed as argument to the constructor
 * is transformed into lower case.
 */
template <typename TaggedElement>
class tagged_element
{
public:

   typedef std::string tag_type;

   tagged_element(const tag_type& tag)
      : _tag(tag)
   {
      boost::algorithm::to_lower(_tag);
   }

   operator const tag_type& () const { return get_tag(); }

   bool operator == (const TaggedElement& elem) const
   { return _tag == elem._tag; }

   bool operator != (const TaggedElement& elem) const
   { return _tag != elem._tag; }

   bool operator < (const TaggedElement& elem) const
   { return _tag < elem._tag; }

   bool operator > (const TaggedElement& elem) const
   { return _tag > elem._tag; }

   const tag_type& get_tag() const { return _tag; }

private:

   tag_type _tag;
};

class variable_group : public tagged_element<variable_group>
{
public:

   variable_group(const tag_type& tag) : tagged_element(tag) {}
};

class variable_name : public tagged_element<variable_name>
{
public:

   variable_name(const tag_type& tag) : tagged_element(tag) {}
};

/**
 * The variable ID, which comprises the var group and the var name.
 */
typedef std::pair<variable_group, variable_name> variable_id;

/**
 * Make a variable ID from its group and name.
 */
inline variable_id make_var_id(
      const variable_group& grp,
      const variable_name& name)
{ return std::make_pair(grp, name); }

/**
 * Make a variable ID from its group and name.
 */
inline variable_id make_var_id(
      const std::string& grp,
      const std::string& name)
{ return std::make_pair(variable_group(grp), variable_name(name)); }

/**
 * Obtain the variable group from the variable ID.
 */
inline variable_group get_var_group(const variable_id& id)
{ return id.first; }

/**
 * Obtain the variable name from the variable ID.
 */
inline variable_name get_var_name(const variable_id& id)
{ return id.second; }

/**
 * Convert variable ID into string.
 */
inline std::string var_to_string(const variable_id& id)
{
   return str(
            boost::format("%s->%s") %
            get_var_group(id).get_tag() %
            get_var_name(id).get_tag());
}

enum variable_type
{
   VAR_BOOLEAN,
   VAR_BYTE,
   VAR_WORD,
   VAR_DWORD,
   VAR_FLOAT
};

class variable_value
{
public:

   /**
    * Thrown when the variable value is used as an invalid type. Contains:
    *   - expected_type_info, indicating the expected type
    *   - actual_type_info, indicating the actual type
    */
   OAC_DECL_ERROR(invalid_type_error, invalid_input_error);

   OAC_DECL_ERROR_INFO(expected_type_info, variable_type);
   OAC_DECL_ERROR_INFO(actual_type_info, variable_type);

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

private:

   variable_type _type;
   ptr<linear_buffer> _buffer;

   inline variable_value(
         const variable_type& type,
         const ptr<linear_buffer>& buffer)
      : _type(type), _buffer(buffer) {}

   void checkType(
         const variable_type& get_type) const throw (invalid_type_error);
};

/**
 * An opaque object which identifies a variable subscription. The internal
 * representation of this type is intended to be opaque to the API consumer.
 */
typedef std::uint32_t subscription_id;

/**
 * Flight vars API interface.
 */
class flight_vars
{
public:      

   /**
    * An operation was requested on a unknown variable.
    */
   OAC_DECL_ERROR(unknown_variable_error, invalid_input_error);

   /**
    * An operation was requested for a unknown variable group. Contains:
    *  - variable_group_info, indicating the var group which was unknown
    */
   OAC_DECL_ERROR(unknown_variable_group_error, unknown_variable_error);

   /**
    * An operation was requested for a unknown variable name. Contains:
    *  - variable_name_info, indicating the var name which was unknown
    */
   OAC_DECL_ERROR(unknown_variable_name_error, unknown_variable_error);

   OAC_DECL_ERROR_INFO(variable_group_info, variable_group);
   OAC_DECL_ERROR_INFO(variable_name_info, variable_name);


   /**
    * A callback representing a subscription to a variable.
    */
   typedef std::function<void(const variable_id& id,
                              const variable_value& value)> var_update_handler;

   virtual ~flight_vars() {}

   /**
    * Subscribe to a variable.
    *
    * @param grp the variable group
    * @param name the variable name
    * @param suhandlerbs the handler to be invoked when var changes.
    * @return the subscription ID, which may be used for unsubscription
    */
   virtual subscription_id subscribe(
         const variable_id& var,
         const var_update_handler& handler) throw (unknown_variable_error) = 0;

   /**
    * Remove the subscription with the given ID. If the given ID is unknown,
    * nothing is done.
    */
   virtual void unsubscribe(const subscription_id& id) = 0;
};

}} // namespace oac::fv

#endif
