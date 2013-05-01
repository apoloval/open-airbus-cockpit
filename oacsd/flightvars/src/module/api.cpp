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
Ptr<Buffer> MakeBufferOf(const T& value)
{
   auto buffer = new FixedBuffer(sizeof(T));
   buffer->writeAs<T>(0, value);
   return buffer;
}

} // anonymous namespace

TaggedElement::TaggedElement(const Tag& tag) : _tag(tag)
{
   boost::algorithm::to_lower(_tag);
}

VariableValue
VariableValue::fromBool(bool value)
{ return VariableValue(VAR_BOOLEAN, MakeBufferOf<bool>(value)); }

VariableValue
VariableValue::fromByte(BYTE value)
{ return VariableValue(VAR_BYTE, MakeBufferOf<BYTE>(value)); }

VariableValue
VariableValue::fromWord(WORD value)
{ return VariableValue(VAR_WORD, MakeBufferOf<WORD>(value)); }

VariableValue
VariableValue::fromDWord(DWORD value)
{ return VariableValue(VAR_DWORD, MakeBufferOf<DWORD>(value)); }

VariableValue
VariableValue::fromFloat(float value)
{ return VariableValue(VAR_FLOAT, MakeBufferOf<float>(value)); }

bool
VariableValue::asBool() const
throw (InvalidTypeError)
{
   checkType(VAR_BOOLEAN);
   return _buffer->readAs<bool>(0);
}

BYTE
VariableValue::asByte() const
throw (InvalidTypeError)
{
   checkType(VAR_BYTE);
   return _buffer->readAs<BYTE>(0);
}

WORD
VariableValue::asWord() const
throw (InvalidTypeError)
{
   checkType(VAR_WORD);
   return _buffer->readAs<WORD>(0);
}

DWORD
VariableValue::asDWord() const
throw (InvalidTypeError)
{
   checkType(VAR_DWORD);
   return _buffer->readAs<DWORD>(0);
}

float
VariableValue::asFloat() const
throw (InvalidTypeError)
{
   checkType(VAR_FLOAT);
   return _buffer->readAs<float>(0);
}

void
VariableValue::checkType(const VariableType& type) const
throw (InvalidTypeError)
{
   if (_type != type)
      THROW_ERROR(InvalidTypeError() <<
            ExpectedTypeInfo(type) << ActualTypeInfo(_type));
}


}} // namespace oac::fv
