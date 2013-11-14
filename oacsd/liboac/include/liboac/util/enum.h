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

#ifndef OAC_UTIL_ENUM_H
#define OAC_UTIL_ENUM_H

#include <string>

#include <liboac/exception.h>

namespace oac { namespace util {

template <typename Enum>
OAC_DECL_EXCEPTION_WITH_PARAMS(enum_value_error, oac::exception,
   ("invalid value %d out of range of enumeration", int(value)),
   (value, Enum)
);

OAC_DECL_EXCEPTION_WITH_PARAMS(enum_tag_error, oac::exception,
   ("invalid tag %s out of range of enumeration", tag),
   (tag, std::string)
);

template <typename Enum>
std::string enum_to_string(
   const Enum& id1,
   const Enum& id2,
   const std::string& tag)
{
   if (id1 == id2)
      return tag;
   else
      OAC_THROW_EXCEPTION(enum_value_error<Enum>(id1));
}

template <typename Enum, typename... Args>
std::string enum_to_string(
   const Enum& id1,
   const Enum& id2,
   const std::string& tag,
   Args... args)
{
   return (id1 == id2) ? tag : enum_to_string(id1, args...);
}

template <typename Enum>
Enum string_to_enum(
   const std::string& tag1,
   const Enum& id,
   const std::string& tag2)
{
   if (tag1 == tag2)
      return id;
   else
      OAC_THROW_EXCEPTION(enum_tag_error(tag1));
}

template <typename Enum, typename... Args>
Enum string_to_enum(
   const std::string& tag1,
   const Enum& id,
   const std::string& tag2,
   Args... args)
{
   return (tag1 == tag2) ? id : string_to_enum(tag1, args...);
}

/**
 * Define a conversions type for an enum type. This macro may be used to
 * create a helper type to perform conversions for an enum type to/from
 * string. The first argument of the macro indicates the name of the enum
 * which conversions type is to be generated. The rest of the arguments
 * are pairs of enum values and tags. E.g.,
 *
 *    enum class colour
 *    {
 *       RED,
 *       GREEN,
 *       BLUE,
 *    };
 *
 *    OAC_DECL_ENUM_CONVERSIONS(colour,
 *       colour::RED, "red",
 *       colour::GREEN, "green",
 *       colour::BLUE, "blue"
 *    );
 *
 * This macro is reduced as:
 *
 *    struct colour_conversions
 *    {
 *       static std::string to_string(const colour& value) { ... }
 *       static colour from_string(const std::string& tag) { ... }
 *    };
 *
 * The to_string() function throws a enum_value_error<colour> if there is
 * no conversion defined for the value passed as argument. That happen if
 * the corresponding tuple was not specified in OAC_DECL_ENUM_CONVERSIONS
 * or the enum value has been forcefully casted from an integer out of range
 * for this enumeration.
 *
 * The from_string() function throws a enum_tag_error if the string passed
 * as argument doesn't match any known value for the enumeration.
 */
#define OAC_DECL_ENUM_CONVERSIONS(enum_class, ...)                            \
   struct enum_class##_conversions                                            \
   {                                                                          \
      static std::string to_string(const enum_class& value)                   \
      {                                                                       \
         return oac::util::enum_to_string(value, __VA_ARGS__);                \
      }                                                                       \
                                                                              \
      static enum_class from_string(const std::string& tag)                   \
      {                                                                       \
         return oac::util::string_to_enum(tag, __VA_ARGS__);                  \
      }                                                                       \
   }

}} // namespace oac::util

#endif
