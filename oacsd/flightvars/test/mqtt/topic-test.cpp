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

#include "mqtt/topic.h"

using namespace oac;
using namespace oac::fv;

BOOST_AUTO_TEST_SUITE(MqttTopicTest)

BOOST_AUTO_TEST_CASE(MustCreateLegalTopic)
{
   mqtt::topic tpc("foo/bar");
   BOOST_CHECK_EQUAL("foo/bar", tpc.to_string());
}

BOOST_AUTO_TEST_CASE(MustThrowOnTopicWithInvalidPlusSymbol)
{
   BOOST_CHECK_THROW(
         mqtt::topic("/foo/+/bar"),
         mqtt::illegal_topic_error);
}

BOOST_AUTO_TEST_CASE(MustThrowOnTopicWithInvalidHashSymbol)
{
   BOOST_CHECK_THROW(
         mqtt::topic("/foo/#/bar"),
         mqtt::illegal_topic_error);
}

BOOST_AUTO_TEST_SUITE_END()


BOOST_AUTO_TEST_SUITE(MqttTopicPatternTest)

BOOST_AUTO_TEST_CASE(MustCreateLegalTopicPattern)
{
   mqtt::topic_pattern pattern("foo/+/bar");
   BOOST_CHECK_EQUAL("foo/+/bar", pattern.to_string());
}

BOOST_AUTO_TEST_CASE(MustThrowOnTopicPatternWithPlusMixedWithOtherSymbols)
{
   BOOST_CHECK_THROW(
         mqtt::topic_pattern("foo/+abc/bar"),
         mqtt::illegal_topic_pattern_error);
}

BOOST_AUTO_TEST_CASE(MustThrowOnTopicPatternWithHashMixedWithOtherSymbols)
{
   BOOST_CHECK_THROW(
         mqtt::topic_pattern("foo/bar#"),
         mqtt::illegal_topic_pattern_error);
}

BOOST_AUTO_TEST_CASE(MustThrowOnTopicPatternWithHashNotInTail)
{
   BOOST_CHECK_THROW(
         mqtt::topic_pattern("foo/#/bar"),
         mqtt::illegal_topic_pattern_error);
}

BOOST_AUTO_TEST_CASE(MustMatchLiteralPattern)
{
   mqtt::topic_pattern pattern("foo/bar");
   BOOST_CHECK(pattern.match(mqtt::topic("foo/bar")));
   BOOST_CHECK(!pattern.match(mqtt::topic("foo/bar/other")));
}

BOOST_AUTO_TEST_CASE(MustMatchPlusWildcard)
{
   mqtt::topic tpc("a/b/c/d");

   BOOST_CHECK(mqtt::topic_pattern("a/b/c/d").match(tpc));
   BOOST_CHECK(mqtt::topic_pattern("+/b/c/d").match(tpc));
   BOOST_CHECK(mqtt::topic_pattern("a/+/c/d").match(tpc));
   BOOST_CHECK(mqtt::topic_pattern("a/+/+/d").match(tpc));
   BOOST_CHECK(mqtt::topic_pattern("+/+/+/+").match(tpc));
   BOOST_CHECK(!mqtt::topic_pattern("a/b/c").match(tpc));
   BOOST_CHECK(!mqtt::topic_pattern("b/+/c/d").match(tpc));
   BOOST_CHECK(!mqtt::topic_pattern("+/+/+").match(tpc));
}

BOOST_AUTO_TEST_CASE(MustMatchHashWildcard)
{
   mqtt::topic tpc("a/b/c/d");

   BOOST_CHECK(mqtt::topic_pattern("a/b/c/d").match(tpc));
   BOOST_CHECK(mqtt::topic_pattern("#").match(tpc));
   BOOST_CHECK(mqtt::topic_pattern("a/#").match(tpc));
   BOOST_CHECK(mqtt::topic_pattern("a/b/#").match(tpc));
   BOOST_CHECK(mqtt::topic_pattern("a/b/c/#").match(tpc));
   BOOST_CHECK(mqtt::topic_pattern("+/b/c/#").match(tpc));
}

BOOST_AUTO_TEST_SUITE_END()
