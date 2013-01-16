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

BOOST_AUTO_TEST_CASE(MaybeShouldEvaluateToNothing)
{
   Maybe<int> m;
   BOOST_CHECK(m.isNothing());
   BOOST_CHECK(!m.isJust());
   BOOST_CHECK(!m);
}

BOOST_AUTO_TEST_CASE(MaybeShouldEvaluateToJustValue)
{
   Maybe<int> m(12);
   BOOST_CHECK(!m.isNothing());
   BOOST_CHECK(m.isJust());
   BOOST_CHECK(m);
   BOOST_CHECK_EQUAL(12, m.get());
   BOOST_CHECK_EQUAL(12, *m);
}

BOOST_AUTO_TEST_CASE(MaybeShouldEvaluateToJustValueAfterSet)
{
   Maybe<int> m;
   m.set(12);
   BOOST_CHECK(!m.isNothing());
   BOOST_CHECK(m.isJust());
   BOOST_CHECK_EQUAL(12, *m);
}

BOOST_AUTO_TEST_CASE(MaybeShouldEvaluateInFunctionChain)
{
   auto f1 = [](int i) -> Maybe<float> { return 3.1416f; };
   auto f2 = [](float f) -> Maybe<std::string> { return std::string("Works!"); };
   Maybe<int> m(7);

   auto r = m >> f1 >> f2;
   BOOST_CHECK_EQUAL("Works!", *r);
}



#include "lang-utils.h"


