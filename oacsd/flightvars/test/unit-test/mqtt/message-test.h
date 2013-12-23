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

#include <boost/test/auto_unit_test.hpp>

#include "mqtt/message.h"

using namespace oac;
using namespace oac::fv;

BOOST_AUTO_TEST_SUITE(MqttMessageTest)

BOOST_AUTO_TEST_CASE(MustConvertFromRawMessage)
{
   int orig_num = 1234;
   mqtt::raw_message orig_msg {
      mqtt::topic { "foo/bar" },
      &orig_num,
      sizeof(int),
      mqtt::qos_level::LEVEL_0,
      false
   };

   auto dest_msg = orig_msg.to_typed<int>();

   BOOST_CHECK_EQUAL("foo/bar", dest_msg.tpc.to_string());
   BOOST_CHECK_EQUAL(1234, dest_msg.data);
   BOOST_CHECK(mqtt::qos_level::LEVEL_0 == dest_msg.qos);
   BOOST_CHECK_EQUAL(false, dest_msg.retain);
}


BOOST_AUTO_TEST_CASE(MustThrowOnConversionFromRawMessageWithInvalidSize)
{
   int orig_num = 1234;
   mqtt::raw_message orig_msg {
      mqtt::topic { "foo/bar" },
      &orig_num,
      sizeof(int),
      mqtt::qos_level::LEVEL_0,
      false
   };

   BOOST_CHECK_THROW(
         orig_msg.to_typed<bool>(),
         mqtt::raw_message::conversion_error);
}
BOOST_AUTO_TEST_SUITE_END()
