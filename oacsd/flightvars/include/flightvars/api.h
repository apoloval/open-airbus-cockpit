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

#include <Windows.h>

#include <liboac/buffer.h>
#include <liboac/exception.h>

namespace oac { namespace fv {

/**
 * A class to represent tagged elements such as var groups, var names, etc.
 * tags are case insensitive. The tag passed as argument to the constructor
 * is transformed into lower case.
 */
class tagged_element
{
public:

   typedef std::string tag;

   tagged_element(const tag& tag);

   inline operator const tag& () const { return get_tag(); }

   inline bool operator == (const tagged_element& elem) const
   { return _tag == elem._tag; }

   inline bool operator != (const tagged_element& elem) const
   { return _tag != elem._tag; }

   inline bool operator < (const tagged_element& elem) const
   { return _tag < elem._tag; }

   inline bool operator > (const tagged_element& elem) const
   { return _tag > elem._tag; }

   inline const tag& get_tag() const { return _tag; }

private:

   tag _tag;
};

class variable_group : public tagged_element
{
public:

   inline variable_group(const tag& tag) : tagged_element(tag) {}
};

class variable_name : public tagged_element
{
public:

   inline variable_name(const tag& tag) : tagged_element(tag) {}
};

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
   static variable_value from_byte(BYTE value);
   static variable_value from_word(WORD value);
   static variable_value from_dword(DWORD value);
   static variable_value from_float(float value);

   inline variable_type get_type() const { return _type; }

   bool as_bool() const throw (invalid_type_error);
   BYTE as_byte() const throw (invalid_type_error);
   WORD as_word() const throw (invalid_type_error);
   DWORD as_dword() const throw (invalid_type_error);
   float as_float() const throw (invalid_type_error);

private:

   variable_type _type;
   ptr<buffer> _buffer;

   inline variable_value(
         const variable_type& type,
         const ptr<buffer>& buffer) : _type(type), _buffer(buffer) {}

   void checkType(const variable_type& get_type) const throw (invalid_type_error);
};

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
   typedef std::function<void(const variable_group& grp,
                              const variable_name& name,
                              const variable_value& value)> subscription;

   /**
    * Subscribe to a variable.
    *
    * @param grp the variable group
    * @param name the variable name
    * @param subs the subscription callback to be invoked when var changes.
    */
   virtual void subscribe(
         const variable_group& grp,
         const variable_name& name,
         const subscription& subs) throw (unknown_variable_error) = 0;
};

}} // namespace oac::fv

#endif
