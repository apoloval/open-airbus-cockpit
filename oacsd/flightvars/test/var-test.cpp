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
 * You Must have received a copy of the GNU General Public License
 * along with Open Airbus Cockpit. If not, see <http://www.gnu.org/licenses/>.
 */

#define BOOST_AUTO_TEST_MAIN
#include <boost/test/auto_unit_test.hpp>

#include <flightvars/var.h>

using namespace oac;
using namespace oac::fv;

BOOST_AUTO_TEST_SUITE(VariableId)

BOOST_AUTO_TEST_CASE(MustCaptureLowerCaseGroup)
{
   variable_id id("my_group/FOOBAR", "my_var/millibars");
   BOOST_CHECK_EQUAL("my_group/foobar", id.group);
   BOOST_CHECK_EQUAL("my_var/millibars", id.name);
}

BOOST_AUTO_TEST_CASE(MustCaptureLowerCaseName)
{
   variable_id id("my_group/foobar", "my_var/MILLIBARS");
   BOOST_CHECK_EQUAL("my_group/foobar", id.group);
   BOOST_CHECK_EQUAL("my_var/millibars", id.name);
}

BOOST_AUTO_TEST_CASE(MustBeEqualWhenGroupAndNameMatches)
{
   variable_id id1("my_group/foobar", "my_var/millibars");
   variable_id id2("my_group/foobar", "my_var/millibars");
   BOOST_CHECK_EQUAL(id1, id2);
}

BOOST_AUTO_TEST_CASE(MustBeDistinctWhenGroupNotMatches)
{
   variable_id id1("my_group/foo", "my_var/millibars");
   variable_id id2("my_group/bar", "my_var/millibars");
   BOOST_CHECK_NE(id1, id2);
}

BOOST_AUTO_TEST_CASE(MustBeDistinctWhenNameNotMatches)
{
   variable_id id1("my_group/foobar", "my_var/millibars");
   variable_id id2("my_group/foobar", "my_var/seconds");
   BOOST_CHECK_NE(id1, id2);
}

BOOST_AUTO_TEST_CASE(MustBeLessThanWhenGroupMatches)
{
   variable_id id1("my_group/foobar", "my_var/1");
   variable_id id2("my_group/foobar", "my_var/2");
   BOOST_CHECK_LT(id1, id2);
}

BOOST_AUTO_TEST_CASE(MustBeLessThanWhenGroupNotMatches)
{
   variable_id id1("my_group/1", "my_var/2");
   variable_id id2("my_group/2", "my_var/1");
   BOOST_CHECK_LT(id1, id2);
}

BOOST_AUTO_TEST_CASE(MustParseFromValidExpression)
{
   auto id = variable_id::parse("my_group/1->my_var/2");
   BOOST_CHECK_EQUAL("my_group/1", id.group);
   BOOST_CHECK_EQUAL("my_var/2", id.name);
}

BOOST_AUTO_TEST_CASE(MustThrowOnParsingFromMissingSeparator)
{
   BOOST_CHECK_THROW(
         variable_id::parse("my_group/1my_var/2"),
         variable_id::parse_error);
}

BOOST_AUTO_TEST_CASE(MustThrowOnParsingFromMissingGroup)
{
   BOOST_CHECK_THROW(
         variable_id::parse("->my_var/2"),
         variable_id::parse_error);
}

BOOST_AUTO_TEST_CASE(MustThrowOnParsingFromMissingName)
{
   BOOST_CHECK_THROW(
         variable_id::parse("my_group/1->"),
         variable_id::parse_error);
}

BOOST_AUTO_TEST_CASE(MustThrowOnParsingFromMoreThanThreeTokens)
{
   BOOST_CHECK_THROW(
         variable_id::parse("my_group/1->my_var/2->my_var/3"),
         variable_id::parse_error);
}

BOOST_AUTO_TEST_SUITE_END()



BOOST_AUTO_TEST_SUITE(VariableValue)

BOOST_AUTO_TEST_CASE(MustMatchCreationAndExtractionTypeForBool)
{
   auto val = variable_value::from_bool(true);
   BOOST_CHECK_EQUAL(variable_type::BOOLEAN, val.get_type());
   BOOST_CHECK_EQUAL(true, val.as_bool());
}

BOOST_AUTO_TEST_CASE(MustMatchCreationAndExtractionTypeForByte)
{
   auto val = variable_value::from_byte(65);
   BOOST_CHECK_EQUAL(variable_type::BYTE, val.get_type());
   BOOST_CHECK_EQUAL(65, val.as_byte());
}

BOOST_AUTO_TEST_CASE(MustMatchCreationAndExtractionTypeForWord)
{
   auto val = variable_value::from_word(1200);
   BOOST_CHECK_EQUAL(variable_type::WORD, val.get_type());
   BOOST_CHECK_EQUAL(1200, val.as_word());
}

BOOST_AUTO_TEST_CASE(MustMatchCreationAndExtractionTypeForDoubleWord)
{
   auto val = variable_value::from_dword(1200);
   BOOST_CHECK_EQUAL(variable_type::DWORD, val.get_type());
   BOOST_CHECK_EQUAL(1200, val.as_dword());
}

BOOST_AUTO_TEST_CASE(MustMatchCreationAndExtractionTypeForFloat)
{
   auto val = variable_value::from_float(3.1416f);
   BOOST_CHECK_EQUAL(variable_type::FLOAT, val.get_type());
   BOOST_CHECK_CLOSE(3.1416f, val.as_float(), 0.0001f);
}

BOOST_AUTO_TEST_CASE(MustFailOnExtractionOfWrongType)
{
   auto val = variable_value::from_dword(1200);
   BOOST_CHECK_THROW(val.as_bool(), variable_value::invalid_type_error);
}

BOOST_AUTO_TEST_CASE(MustConvertToStringForBool)
{
   auto val1 = variable_value::from_bool(true);
   auto val2 = variable_value::from_bool(false);
   BOOST_CHECK_EQUAL("true(bool)", val1.to_string());
   BOOST_CHECK_EQUAL("false(bool)", val2.to_string());
}

BOOST_AUTO_TEST_CASE(MustConvertToStringForByte)
{
   auto val = variable_value::from_byte(32);
   BOOST_CHECK_EQUAL("32(byte)", val.to_string());
}

BOOST_AUTO_TEST_CASE(MustConvertToStringForWord)
{
   auto val = variable_value::from_word(1001);
   BOOST_CHECK_EQUAL("1001(word)", val.to_string());
}

BOOST_AUTO_TEST_CASE(MustConvertToStringForDoubleWord)
{
   auto val = variable_value::from_dword(1001);
   BOOST_CHECK_EQUAL("1001(dword)", val.to_string());
}

BOOST_AUTO_TEST_CASE(MustConvertToStringForFloat)
{
   auto val = variable_value::from_float(3.1415f);
   BOOST_CHECK_EQUAL("3.141500(float)", val.to_string());
}

BOOST_AUTO_TEST_SUITE_END()
