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
 * Tags are case insensitive. The tag passed as argument to the constructor
 * is transformed into lower case.
 */
class TaggedElement
{
public:

   typedef std::string Tag;

   TaggedElement(const Tag& tag);

   inline operator const Tag& () const { return tag(); }

   inline bool operator == (const TaggedElement& elem) const
   { return _tag == elem._tag; }

   inline bool operator != (const TaggedElement& elem) const
   { return _tag != elem._tag; }

   inline bool operator < (const TaggedElement& elem) const
   { return _tag < elem._tag; }

   inline bool operator > (const TaggedElement& elem) const
   { return _tag > elem._tag; }

   inline const Tag& tag() const { return _tag; }

private:

   Tag _tag;
};

class VariableGroup : public TaggedElement
{
public:

   inline VariableGroup(const Tag& tag) : TaggedElement(tag) {}
};

class VariableName : public TaggedElement
{
public:

   inline VariableName(const Tag& tag) : TaggedElement(tag) {}
};

enum VariableType
{
   VAR_BOOLEAN,
   VAR_BYTE,
   VAR_WORD,
   VAR_DWORD,
   VAR_FLOAT
};

class VariableValue
{
public:

   /**
    * Thrown when the variable value is used as an invalid type. Contains:
    *   - ExpectedTypeInfo, indicating the expected type
    *   - ActualTypeInfo, indicating the actual type
    */
   DECL_ERROR(InvalidTypeError, InvalidInputError);

   DECL_ERROR_INFO(ExpectedTypeInfo, VariableType);
   DECL_ERROR_INFO(ActualTypeInfo, VariableType);

   static VariableValue fromBool(bool value);
   static VariableValue fromByte(BYTE value);
   static VariableValue fromWord(WORD value);
   static VariableValue fromDWord(DWORD value);
   static VariableValue fromFloat(float value);

   inline VariableType type() const { return _type; }

   bool asBool() const throw (InvalidTypeError);
   BYTE asByte() const throw (InvalidTypeError);
   WORD asWord() const throw (InvalidTypeError);
   DWORD asDWord() const throw (InvalidTypeError);
   float asFloat() const throw (InvalidTypeError);

private:

   VariableType _type;
   Ptr<Buffer> _buffer;

   inline VariableValue(
         const VariableType& type,
         const Ptr<Buffer>& buffer) : _type(type), _buffer(buffer) {}

   void checkType(const VariableType& type) const throw (InvalidTypeError);
};

/**
 * Flight vars API interface.
 */
class FlightVars
{
public:      

   /**
    * An operation was requested on a unknown variable.
    */
   DECL_ERROR(UnknownVariableError, InvalidInputError);

   /**
    * An operation was requested for a unknown variable group. Contains:
    *  - VariableGroupInfo, indicating the var group which was unknown
    */
   DECL_ERROR(UnknownVariableGroupError, UnknownVariableError);

   /**
    * An operation was requested for a unknown variable name. Contains:
    *  - VariableNameInfo, indicating the var name which was unknown
    */
   DECL_ERROR(UnknownVariableNameError, UnknownVariableError);

   DECL_ERROR_INFO(VariableGroupInfo, VariableGroup);
   DECL_ERROR_INFO(VariableNameInfo, VariableName);

   /**
    * A callback representing a subscription to a variable.
    */
   typedef std::function<void(const VariableGroup& grp,
                              const VariableName& name,
                              const VariableValue& value)> Subscription;

   /**
    * Subscribe to a variable.
    *
    * @param grp the variable group
    * @param name the variable name
    * @param subs the subscription callback to be invoked when var changes.
    */
   virtual void subscribe(
         const VariableGroup& grp,
         const VariableName& name,
         const Subscription& subs) throw (UnknownVariableError) = 0;
};

}} // namespace oac::fv

#endif
