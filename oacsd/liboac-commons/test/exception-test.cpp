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

#define BOOST_AUTO_TEST_MAIN
#include <boost/test/auto_unit_test.hpp>

#include "exception.h"

using namespace oac;

#define RELATIVE_LINE(n) (__LINE__ + (n))

OAC_ABSTRACT_EXCEPTION(abstract_exception);

BOOST_AUTO_TEST_SUITE(SimpleException)

OAC_EXCEPTION(
      simple_exception,
      abstract_exception,
      "Simple error occurred");

OAC_EXCEPTION(
      other_simple_exception,
      abstract_exception,
      "Other simple error occurred");

BOOST_AUTO_TEST_CASE(MustHonorErrorMessage)
{
   try { OAC_THROW_EXCEPTION(simple_exception()); }
   catch (simple_exception& e)
   {
      BOOST_CHECK_EQUAL(
            "Simple error occurred",
            e.message());
   }
}

BOOST_AUTO_TEST_CASE(MustReportErrorMessageWhenMissingCause)
{
   try { OAC_THROW_EXCEPTION(simple_exception()); }
   catch (simple_exception& e)
   {
      BOOST_CHECK_EQUAL(
            format(
                  "in %s:%d, %s",
                  __FILE__,
                  RELATIVE_LINE(-9),
                  "Simple error occurred"),
            e.report());
   }
}

BOOST_AUTO_TEST_CASE(MustReportWhenItHasCause)
{
   try { OAC_THROW_EXCEPTION(simple_exception()); }
   catch (simple_exception& e1)
   {
      try { OAC_THROW_EXCEPTION(other_simple_exception().with_cause(e1)); }
      catch (other_simple_exception& e2)
      {
         auto msg1 = format(
               "in %s:%d, %s",
               __FILE__,
               RELATIVE_LINE(-6),
               "Other simple error occurred");
         auto msg2 = format(
               "in %s:%d, %s",
               __FILE__,
               RELATIVE_LINE(-14),
               "Simple error occurred");
         BOOST_CHECK_EQUAL(
               format("%s; caused by:\n%s", msg1, msg2),
               e2.report());
      }
   }
}

BOOST_AUTO_TEST_SUITE_END()



BOOST_AUTO_TEST_SUITE(ComplexException)

OAC_EXCEPTION_BEGIN(complex_exception, abstract_exception)
   OAC_EXCEPTION_FIELD(number, int)
   OAC_EXCEPTION_FIELD(name, std::string)
   OAC_EXCEPTION_MSG("Complex error with number %d and name %s", number, name)
OAC_EXCEPTION_END()

BOOST_AUTO_TEST_CASE(MustHoldParameters)
{
   try
   {
      OAC_THROW_EXCEPTION(
            complex_exception()
                  .with_name("apv")
                  .with_number(7));
   }
   catch (complex_exception& e)
   {
      BOOST_CHECK_EQUAL("apv", e.get_name());
      BOOST_CHECK_EQUAL(7, e.get_number());
   }
}

BOOST_AUTO_TEST_CASE(MustReportExpectedMessage)
{
   try
   {
      OAC_THROW_EXCEPTION(
            complex_exception()
                  .with_name("apv")
                  .with_number(7));
   }
   catch (complex_exception& e)
   {
      BOOST_CHECK_EQUAL(
            format(
                  "in %s:%d, %s",
                  __FILE__,
                  RELATIVE_LINE(-10),
                  "Complex error with number 7 and name apv"),
            e.report());
   }
}

BOOST_AUTO_TEST_SUITE_END()
