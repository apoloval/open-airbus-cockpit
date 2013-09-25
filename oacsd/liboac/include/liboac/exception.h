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

#include <boost/preprocessor.hpp>

#include <liboac/format.h>

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

   template <typename... Args>
   exception(const char* fmt, const Args&... args)
      : _source("unknown source"),
        _msg(format(fmt, args...)),
        _cause(nullptr)
   {}

   template <typename Exception, typename... Args>
   exception(const Exception& cause, const char* fmt, const Args&... args)
      : _source("unknown source"),
        _msg(format(fmt, args...)),
        _cause(std::make_shared<Exception>(cause))
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

#define OAC_DECL_EXCEPTION(class_name, parent_class, msg)                  \
   class class_name : public parent_class                                  \
   {                                                                       \
   public:                                                                 \
                                                                           \
      class_name() : parent_class(msg)                                     \
      {}                                                                   \
                                                                           \
      template <typename Exception>                                        \
      class_name(const Exception& cause) : parent_class(cause, msg)        \
      {}                                                                   \
                                                                           \
      class_name& set_source(const std::string& source)                    \
      { _set_source(source); return *this; }                               \
   }

#define OAC_EXCEPTION_CTOR_PARAM(r, data, i, param)                        \
   BOOST_PP_COMMA_IF(i)                                                    \
   const BOOST_PP_TUPLE_ELEM(2, 1, param)&                                 \
   BOOST_PP_TUPLE_ELEM(2, 0, param)

#define OAC_EXCEPTION_CTOR_PARAMS(...)                                     \
   BOOST_PP_LIST_FOR_EACH_I(                                               \
      OAC_EXCEPTION_CTOR_PARAM,                                            \
      _,                                                                   \
      BOOST_PP_VARIADIC_TO_LIST(__VA_ARGS__))

#define OAC_EXCEPTION_INIT_FIELD(r, data, i, param)                        \
   BOOST_PP_COMMA_IF(i)                                                    \
   BOOST_PP_CAT(_, BOOST_PP_TUPLE_ELEM(2, 0, param))                       \
   (BOOST_PP_TUPLE_ELEM(2, 0, param))

#define OAC_EXCEPTION_INIT_FIELDS(...)                                     \
   BOOST_PP_LIST_FOR_EACH_I(                                               \
      OAC_EXCEPTION_INIT_FIELD,                                            \
      _,                                                                   \
      BOOST_PP_VARIADIC_TO_LIST(__VA_ARGS__))

#define OAC_EXCEPTION_DECL_FIELD(r, data, param)                           \
   BOOST_PP_TUPLE_ELEM(2, 1, param)                                        \
   BOOST_PP_CAT(_, BOOST_PP_TUPLE_ELEM(2, 0, param));

#define OAC_EXCEPTION_DECL_FIELDS(...)                                     \
   BOOST_PP_LIST_FOR_EACH(                                                 \
      OAC_EXCEPTION_DECL_FIELD,                                            \
      _,                                                                   \
      BOOST_PP_VARIADIC_TO_LIST(__VA_ARGS__))

#define OAC_EXCEPTION_DECL_GETTER(r, data, param)                          \
   const BOOST_PP_TUPLE_ELEM(2, 1, param)&                                 \
   BOOST_PP_CAT(get_, BOOST_PP_TUPLE_ELEM(2, 0, param))() const            \
   { return BOOST_PP_CAT(_, BOOST_PP_TUPLE_ELEM(2, 0, param)); }

#define OAC_EXCEPTION_DECL_GETTERS(...)                                    \
   BOOST_PP_LIST_FOR_EACH(                                                 \
      OAC_EXCEPTION_DECL_GETTER,                                           \
      _,                                                                   \
      BOOST_PP_VARIADIC_TO_LIST(__VA_ARGS__))


#define OAC_DECL_EXCEPTION_WITH_PARAMS(class_name, parent_class, msg, ...) \
   class class_name : public parent_class                                  \
   {                                                                       \
   public:                                                                 \
                                                                           \
      class_name(OAC_EXCEPTION_CTOR_PARAMS(__VA_ARGS__))                   \
         : parent_class(BOOST_PP_TUPLE_REM_CTOR(msg)),                     \
           OAC_EXCEPTION_INIT_FIELDS(__VA_ARGS__)                          \
      {}                                                                   \
                                                                           \
      template <typename Exception>                                        \
      class_name(                                                          \
            OAC_EXCEPTION_CTOR_PARAMS(__VA_ARGS__),                        \
            const Exception& cause)                                        \
         : parent_class(cause, BOOST_PP_TUPLE_REM_CTOR(msg)),              \
           OAC_EXCEPTION_INIT_FIELDS(__VA_ARGS__)                          \
      {}                                                                   \
                                                                           \
      OAC_EXCEPTION_DECL_GETTERS(__VA_ARGS__)                              \
                                                                           \
      class_name& set_source(const std::string& source)                    \
      { _set_source(source); return *this; }                               \
                                                                           \
   private:                                                                \
                                                                           \
      OAC_EXCEPTION_DECL_FIELDS(__VA_ARGS__)                               \
   }

#define OAC_DECL_ABSTRACT_EXCEPTION(class_name)                            \
   struct class_name : ::oac::exception                                    \
   {                                                                       \
      class_name(const class_name& e)                                      \
         : ::oac::exception(e)                                             \
      {}                                                                   \
                                                                           \
   protected:                                                              \
                                                                           \
      template <typename... Args>                                          \
      class_name(const char* fmt, const Args&... args)                     \
         : ::oac::exception(fmt, args...)                                  \
      {}                                                                   \
                                                                           \
      template <typename Exception, typename... Args>                      \
      class_name(                                                          \
            const Exception& cause,                                        \
            const char* fmt, const                                         \
            Args&... args)                                                 \
         : ::oac::exception(cause, fmt, args...)                           \
      {}                                                                   \
                                                                           \
      template <typename Exception>                                        \
      class_name(const Exception& cause)                                   \
         : ::oac::exception(cause)                                         \
      {}                                                                   \
   }

#define OAC_MAKE_EXCEPTION(...)                                \
   (__VA_ARGS__)                                               \
         .set_source(format("%s:%d", __FILE__, __LINE__))

#define OAC_THROW_EXCEPTION(...)                               \
   throw OAC_MAKE_EXCEPTION(__VA_ARGS__)



/**
 * An exception caused by a enum value out of its range.
 */
template <typename Enum>
OAC_DECL_EXCEPTION_WITH_PARAMS(enum_out_of_range_error, oac::exception,
   ("invalid value %d out of range of enumeration", int(value)),
   (value, Enum));

} // namespace oac

#endif
