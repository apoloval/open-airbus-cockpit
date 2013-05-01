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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Open Airbus Cockpit. If not, see <http://www.gnu.org/licenses/>.
 */

#define BOOST_AUTO_TEST_MAIN
#include <boost/test/auto_unit_test.hpp>

#include "api.h"

using namespace oac;
using namespace oac::fv;

BOOST_AUTO_TEST_SUITE(VariableGroupTest)

BOOST_AUTO_TEST_CASE(ShouldCreateGroupAsLowerCase)
{
   VariableGroup grp("MY_GROUP/FooBar");
   BOOST_CHECK_EQUAL("my_group/foobar", grp.tag());
}

BOOST_AUTO_TEST_SUITE_END()



BOOST_AUTO_TEST_SUITE(VariableNameTest)

BOOST_AUTO_TEST_CASE(ShouldCreateNameAsLowerCase)
{
   VariableName name("MY_VAR/Millibars");
   BOOST_CHECK_EQUAL("my_var/millibars", name.tag());
}

BOOST_AUTO_TEST_SUITE_END()



BOOST_AUTO_TEST_SUITE(VariableValueTest)

BOOST_AUTO_TEST_CASE(ShouldMatchCreationAndExtractionType)
{
   auto val = VariableValue::fromDWord(1200);
   BOOST_CHECK_EQUAL(VAR_DWORD, val.type());
   BOOST_CHECK_EQUAL(1200, val.asDWord());
}

BOOST_AUTO_TEST_CASE(ShouldFailOnExtractionOfWrongType)
{
   auto val = VariableValue::fromDWord(1200);
   BOOST_CHECK_THROW(val.asBool(), VariableValue::InvalidTypeError);
}

BOOST_AUTO_TEST_SUITE_END()
