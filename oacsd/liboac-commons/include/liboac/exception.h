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

#ifndef OAC_EXCEPTION_H
#define OAC_EXCEPTION_H

#include <exception>
#include <string>

#include "format.h"

#pragma warning( disable : 4290 )

namespace oac {

/**
 * Exception base class. This class provides a common base for all exceptions
 * managed in OAC.
 */
class exception : public std::exception
{
public:

   exception(const exception& e)
      : _source(e._source),
        _msg(e._msg),
        _cause(e._cause)
   {}

   virtual ~exception() {}

   /**
    * Obtain the error message for this exception.
    */
   std::string message() const
   { return _msg; }

   /**
    * Obtain a report of this exception. The report contains the
    * error message of this exception and its chain of causes, if any.
    */
   std::string report() const
   {
      if (_cause)
      {
         auto oac_cause = dynamic_cast<exception*>(_cause.get());
         return format(
               "in %s, %s; caused by:\n%s",
               _source,
               _msg,
               oac_cause ? oac_cause->report() : _cause->what());
      }
      else
         return format("in %s, %s", _source, _msg);
   }

   /**
    * Obtain the cause cause exception, if any.
    */
   std::weak_ptr<std::exception> cause() const
   { return std::weak_ptr<std::exception>(_cause); }

   virtual const char* what() const throw()
   { return _msg.c_str(); }

protected:

   exception()
      : _source("unknown source"),
        _msg("Unknown error"),
        _cause(nullptr)
   {}

   void _set_source(const std::string& source)
   { _source = source; }

   void _set_message(const std::string& msg)
   { _msg = msg; }

   template <typename Exception>
   void _set_cause(const Exception& cause)
   { _cause = std::make_shared<Exception>(cause); }

private:

   std::string _source;
   std::string _msg;
   std::shared_ptr<std::exception> _cause;
};

#define _OAC_EXCEPTION_SETTERS(class_name)                     \
         class_name& set_source(const std::string& source)     \
         {                                                     \
            _set_source(source);                               \
            return *this;                                      \
         }                                                     \
                                                               \
         template <typename Exception>                         \
         class_name& with_cause(const Exception& e)            \
         {                                                     \
            _set_cause(e);                                     \
            return *this;                                      \
         }

#define OAC_EXCEPTION_BEGIN(class_name, parent)                \
      class class_name : public parent                         \
      {                                                        \
      public:                                                  \
         typedef class_name __this_type;                       \
                                                               \
         _OAC_EXCEPTION_SETTERS(class_name)


#define OAC_EXCEPTION_MSG(...)                                 \
         void regen_message()                                  \
         {                                                     \
            _set_message(format(__VA_ARGS__));                 \
         }                                                     \

#define OAC_EXCEPTION_FIELD(name, type)                        \
      private:                                                 \
         type name;                                            \
      public:                                                  \
         __this_type& with_##name(const type& _v)              \
         {                                                     \
            name = _v;                                         \
            regen_message();                                   \
            return *this;                                      \
         }                                                     \
                                                               \
         const type& get_##name() const                        \
         { return name; }

#define OAC_EXCEPTION_END()                                    \
      };

#define OAC_ABSTRACT_EXCEPTION(class_name)                     \
      struct class_name : ::oac::exception                     \
      {                                                        \
         class_name(const class_name& e)                       \
            : ::oac::exception(e)                              \
         {}                                                    \
                                                               \
      protected:                                               \
                                                               \
         class_name() : ::oac::exception() {}                  \
                                                               \
         template <typename Exception>                         \
         class_name(const Exception& cause)                    \
            : ::oac::exception(cause)                          \
         {}                                                    \
      }

#define OAC_EXCEPTION(class_name, parent, error_msg)           \
      struct class_name : parent                               \
      {                                                        \
         class_name() : parent()                               \
         { _set_message(error_msg); }                          \
         _OAC_EXCEPTION_SETTERS(class_name)                    \
      }

#define OAC_THROW_EXCEPTION(...)                               \
   throw (__VA_ARGS__)                                         \
         .set_source(format("%s:%d", __FILE__, __LINE__))



/**
 * An exception caused by a enum value out of its range.
 */
template <typename Enum>
OAC_EXCEPTION_BEGIN(enum_out_of_range_error, oac::exception)
   OAC_EXCEPTION_FIELD(value, Enum)
   OAC_EXCEPTION_MSG(
         "invalid value %d out of range of enumeration",
         int(value))
OAC_EXCEPTION_END()

} // namespace oac

#endif
