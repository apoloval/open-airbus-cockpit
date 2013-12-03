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

#include <boost/test/auto_unit_test.hpp>

#include <liboac/attempt.h>

using namespace oac;

BOOST_AUTO_TEST_SUITE(AttemptTest)

OAC_DECL_EXCEPTION(
      fake_error,
      oac::exception,
      "A fake error for testing purposes");

BOOST_AUTO_TEST_CASE(MustGetFromSuccessAttempt)
{
   attempt<int> a(12);

   BOOST_CHECK_EQUAL(12, a.get_value());
}

BOOST_AUTO_TEST_CASE(MustThrowFromErroneousAttempt)
{
   auto error = OAC_MAKE_EXCEPTION(fake_error());
   attempt<int> a(error);

   BOOST_CHECK_THROW(
         a.get_value(),
         fake_error);
}

BOOST_AUTO_TEST_CASE(MustCopyConstructFromSuccessAttempt)
{
   attempt<int> a(12);
   attempt<int> b(a);

   BOOST_CHECK_EQUAL(12, b.get_value());
}

BOOST_AUTO_TEST_CASE(MustCopyConstructFromErroneousAttempt)
{
   auto error = OAC_MAKE_EXCEPTION(fake_error());
   attempt<int> a(error);
   attempt<int> b(a);

   BOOST_CHECK_THROW(
         b.get_value(),
         fake_error);
}

BOOST_AUTO_TEST_CASE(MustAssignFromSuccessAttempt)
{
   attempt<int> a(12);
   auto b = a;

   BOOST_CHECK_EQUAL(12, b.get_value());
}

BOOST_AUTO_TEST_CASE(MustAssignFromErroneousAttempt)
{
   auto error = OAC_MAKE_EXCEPTION(fake_error());
   attempt<int> a(error);
   auto b = a;

   BOOST_CHECK_THROW(
         b.get_value(),
         fake_error);
}

BOOST_AUTO_TEST_SUITE_END()
