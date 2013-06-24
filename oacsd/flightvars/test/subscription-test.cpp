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

#include "subscription.h"

using namespace oac;
using namespace oac::fv;

BOOST_AUTO_TEST_SUITE(SubscriptionMapTest)

BOOST_AUTO_TEST_CASE(ShouldRegisterSubscription)
{
   subscription_mapper mapper;
   auto var_id = make_var_id("fsuipc/offset", "0x4ca1");
   auto subs_id = make_subscription_id();
   mapper.register_subscription(var_id, subs_id);
   BOOST_CHECK_EQUAL(
            "fsuipc/offset",
            get_var_group(mapper.get_var_id(subs_id)).get_tag());
   BOOST_CHECK_EQUAL(
            "0x4ca1",
            get_var_name(mapper.get_var_id(subs_id)).get_tag());
   BOOST_CHECK_EQUAL(subs_id, mapper.get_subscription_id(var_id));
}

BOOST_AUTO_TEST_CASE(ShouldThrowOnRegisterOnExistingVarId)
{
   subscription_mapper mapper;
   auto var_id = make_var_id("fsuipc/offset", "0x4ca1");
   auto subs_id1 = make_subscription_id();
   auto subs_id2 = make_subscription_id();

   mapper.register_subscription(var_id, subs_id1);
   BOOST_CHECK_THROW(
            mapper.register_subscription(var_id, subs_id2),
            subscription_mapper::duplicated_variable_error);
}

BOOST_AUTO_TEST_CASE(ShouldThrowOnRegisterOnExistingSubscriptionId)
{
   subscription_mapper mapper;
   auto var_id1 = make_var_id("fsuipc/offset", "0x4ca1");
   auto var_id2 = make_var_id("fsuipc/offset", "0x89ab");
   auto subs_id = make_subscription_id();

   mapper.register_subscription(var_id1, subs_id);
   BOOST_CHECK_THROW(
            mapper.register_subscription(var_id2, subs_id),
            subscription_mapper::duplicated_subscription_error);
}

BOOST_AUTO_TEST_CASE(ShouldUnregisterSubscriptionByVarId)
{
   subscription_mapper mapper;
   auto var_id = make_var_id("fsuipc/offset", "0x4ca1");
   auto subs_id = make_subscription_id();
   mapper.register_subscription(var_id, subs_id);
   mapper.unregister(var_id);
   BOOST_CHECK_THROW(
            mapper.get_var_id(subs_id),
            subscription_mapper::unknown_subscription_error);
   BOOST_CHECK_THROW(
            mapper.get_subscription_id(var_id),
            subscription_mapper::unknown_variable_error);
}

BOOST_AUTO_TEST_CASE(ShouldUnregisterSubscriptionBySubscriptionId)
{
   subscription_mapper mapper;
   auto var_id = make_var_id("fsuipc/offset", "0x4ca1");
   auto subs_id = make_subscription_id();
   mapper.register_subscription(var_id, subs_id);
   mapper.unregister(subs_id);
   BOOST_CHECK_THROW(
            mapper.get_var_id(subs_id),
            subscription_mapper::unknown_subscription_error);
   BOOST_CHECK_THROW(
            mapper.get_subscription_id(var_id),
            subscription_mapper::unknown_variable_error);
}

BOOST_AUTO_TEST_SUITE_END()