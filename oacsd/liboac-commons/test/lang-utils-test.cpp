/*
 * This file is part of Open Airbus Cockpit
 * Copyright (C) 2012 Alvaro Polo
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

#include <string>

#define BOOST_AUTO_TEST_MAIN
#include <boost/test/auto_unit_test.hpp>

#include "lang-utils.h"

using namespace oac;

maybe<float> f1(int i)
{
   return (i <= 10 && i >= 0) ? 3.1416f : maybe<float>::NOTHING;
}

maybe<std::string> f2(float f)
{
   return std::string("Works!");
}

BOOST_AUTO_TEST_CASE(PointerShouldConstructAsNull)
{
   ptr<int> p;
   BOOST_CHECK(!p);
}

BOOST_AUTO_TEST_CASE(PointerShouldConstructWithValue)
{
   ptr<int> p(new int(12));
   BOOST_CHECK(p);
   BOOST_CHECK_EQUAL(12, *p);
}

BOOST_AUTO_TEST_CASE(PointerShouldConstructByCopy)
{
   ptr<int> p1(new int(12));
   ptr<int> p2(p1);
   BOOST_CHECK(p2);
   BOOST_CHECK_EQUAL(12, *p2);
}

BOOST_AUTO_TEST_CASE(PointerShouldConstructByCopyFromStlType)
{
   std::shared_ptr<int> p1(new int(12));
   ptr<int> p2(p1);
   BOOST_CHECK(p2);
   BOOST_CHECK_EQUAL(12, *p2);
}

BOOST_AUTO_TEST_CASE(MaybeShouldEvaluateToNothing)
{
   maybe<int> m;
   BOOST_CHECK(m.is_nothing());
   BOOST_CHECK(!m.is_just());
   BOOST_CHECK_THROW(m.get(), illegal_state_error);
   BOOST_CHECK_THROW(*m, illegal_state_error);
}

BOOST_AUTO_TEST_CASE(MaybeShouldEvaluateToJustValue)
{
   maybe<int> m(12);
   BOOST_CHECK(!m.is_nothing());
   BOOST_CHECK(m.is_just());
   BOOST_CHECK_EQUAL(12, m.get());
   BOOST_CHECK_EQUAL(12, *m);
}

BOOST_AUTO_TEST_CASE(MaybeShouldEvaluateToJustValueAfterSet)
{
   maybe<int> m;
   m.set(12);
   BOOST_CHECK(!m.is_nothing());
   BOOST_CHECK(m.is_just());
   BOOST_CHECK_EQUAL(12, *m);
}

BOOST_AUTO_TEST_CASE(MaybeShouldEvaluateToJustInSuccessFunctionChain)
{
   maybe<int> m(7);
   std::string result;

   BOOST_CHECK(m >> f1 >> f2 >> result);
   BOOST_CHECK_EQUAL("Works!", result);
}

BOOST_AUTO_TEST_CASE(MaybeShouldEvaluateToNothingInBrokenFunctionChain)
{
   maybe<int> m(15);
   std::string result;

   BOOST_CHECK(!(m >> f1 >> f2 >> result));
}
