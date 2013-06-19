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
