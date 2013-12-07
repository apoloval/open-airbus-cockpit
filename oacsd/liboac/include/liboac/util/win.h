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

#ifndef OAC_UTIL_WIN_H
#define OAC_UTIL_WIN_H

#include <Windows.h>

#include <liboac/exception.h>

namespace oac { namespace util {

OAC_DECL_EXCEPTION_WITH_PARAMS(win32_exception, oac::exception,
   ("win32 function %s returned with error code %d", func_name, error_code),
   (func_name, std::string),
   (error_code, int)
);

template <typename Action, typename ResultPredicate>
auto exec_win32_function(
      const std::string& func_name,
      Action& action,
      ResultPredicate& pred) -> decltype(action())
{
   auto result = action();
   if (!pred(result))
   {
      auto ec = GetLastError();
      OAC_THROW_EXCEPTION(win32_exception(func_name, ec));
   }
   return result;
}

/**
 * Invoke a Win32 API function from an object expecting to have the
 * given result. If the result of the API function is not the expected one,
 * a win32_exception is thrown indicating the function that failed and
 * the system error code that caused the failure.
 */
#define OAC_CALL_WIN32_EXPECT(expected_result, func, ...)                  \
   oac::util::exec_win32_function(                                         \
         #func,                                                            \
         [&]()                                                             \
         {                                                                 \
            return func(__VA_ARGS__);                                      \
         },                                                                \
         [](const decltype(func(__VA_ARGS__))& actual_result)              \
         {                                                                 \
            return actual_result == expected_result;                       \
         }                                                                 \
   )

/**
 * Invoke a Win32 API function from an object expecting not to have the
 * given result. If the result of the API function is the unexpected one,
 * a win32_exception is thrown indicating the function that failed and
 * the system error code that caused the failure.
 */
#define OAC_CALL_WIN32_EXPECT_NOT(unexpected_result, func, ...)            \
   oac::util::exec_win32_function(                                         \
         #func,                                                            \
         [&]()                                                             \
         {                                                                 \
            return func(__VA_ARGS__);                                      \
         },                                                                \
         [](const decltype(func(__VA_ARGS__))& actual_result)              \
         {                                                                 \
            return actual_result != unexpected_result;                     \
         }                                                                 \
   )

/**
 * Invoke a Win32 API function from an object expecting not to have NULL
 * as result. If the result of the API function is NULL, a win32_exception
 * is thrown indicating the function that failed and the system error code
 * that caused the failure.
 */
#define OAC_CALL_WIN32_EXPECT_NOT_NULL(func, ...)                          \
   OAC_CALL_WIN32_EXPECT_NOT(NULL, func, __VA_ARGS__)

}} // namespace oac::util


#endif
