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

#include <set>

#define BOOST_AUTO_TEST_MAIN
#include <boost/test/auto_unit_test.hpp>

#include "subscription.h"

using namespace oac;
using namespace oac::fv;

BOOST_AUTO_TEST_SUITE(SubscriptionMapTest)

BOOST_AUTO_TEST_CASE(ShouldRegisterSubscription)
{
   subscription_mapper mapper;
   variable_id var_id("fsuipc/offset", "0x4ca1");
   auto subs_id = make_subscription_id();
   mapper.register_subscription(var_id, subs_id);
   BOOST_CHECK_EQUAL(
            "fsuipc/offset",
            mapper.get_var_id(subs_id).group);
   BOOST_CHECK_EQUAL(
            "0x4ca1",
            mapper.get_var_id(subs_id).name);
   BOOST_CHECK_EQUAL(subs_id, mapper.get_subscription_id(var_id));
}

BOOST_AUTO_TEST_CASE(ShouldThrowOnRegisterOnExistingVarId)
{
   subscription_mapper mapper;
   variable_id var_id("fsuipc/offset", "0x4ca1");
   auto subs_id1 = make_subscription_id();
   auto subs_id2 = make_subscription_id();

   mapper.register_subscription(var_id, subs_id1);
   BOOST_CHECK_THROW(
            mapper.register_subscription(var_id, subs_id2),
            subscription_mapper::variable_already_exists_error);
}

BOOST_AUTO_TEST_CASE(ShouldThrowOnRegisterOnExistingSubscriptionId)
{
   subscription_mapper mapper;
   auto var_id1 = variable_id("fsuipc/offset", "0x4ca1");
   auto var_id2 = variable_id("fsuipc/offset", "0x89ab");
   auto subs_id = make_subscription_id();

   mapper.register_subscription(var_id1, subs_id);
   BOOST_CHECK_THROW(
            mapper.register_subscription(var_id2, subs_id),
            subscription_mapper::subscription_already_exists_error);
}

BOOST_AUTO_TEST_CASE(ShouldUnregisterSubscriptionByVarId)
{
   subscription_mapper mapper;
   variable_id var_id("fsuipc/offset", "0x4ca1");
   auto subs_id = make_subscription_id();
   mapper.register_subscription(var_id, subs_id);
   mapper.unregister(var_id);
   BOOST_CHECK_THROW(
            mapper.get_var_id(subs_id),
            subscription_mapper::no_such_subscription_error);
   BOOST_CHECK_THROW(
            mapper.get_subscription_id(var_id),
            subscription_mapper::no_such_variable_error);
}

BOOST_AUTO_TEST_CASE(ShouldUnregisterSubscriptionBySubscriptionId)
{
   subscription_mapper mapper;
   variable_id var_id("fsuipc/offset", "0x4ca1");
   auto subs_id = make_subscription_id();
   mapper.register_subscription(var_id, subs_id);
   mapper.unregister(subs_id);
   BOOST_CHECK_THROW(
            mapper.get_var_id(subs_id),
            subscription_mapper::no_such_subscription_error);
   BOOST_CHECK_THROW(
            mapper.get_subscription_id(var_id),
            subscription_mapper::no_such_variable_error);
}

BOOST_AUTO_TEST_CASE(ShouldExecuteActionForEachSubscription)
{
   subscription_mapper mapper;
   variable_id var_id[3] =
   {
      variable_id("fsuipc/offset", "0x4ca1"),
      variable_id("fsuipc/offset", "0x4ca2"),
      variable_id("fsuipc/offset", "0x4ca3"),
   };
   subscription_id subs_id[3] =
   {
      make_subscription_id(),
      make_subscription_id(),
      make_subscription_id(),
   };
   std::set<subscription_id> subs_traversed;

   mapper.register_subscription(var_id[0], subs_id[0]);
   mapper.register_subscription(var_id[1], subs_id[1]);
   mapper.register_subscription(var_id[2], subs_id[2]);

   mapper.for_each_subscription([&subs_traversed](const subscription_id& subs)
   {
      subs_traversed.insert(subs);
   });
   BOOST_CHECK(subs_traversed.find(subs_id[0]) != subs_traversed.end());
   BOOST_CHECK(subs_traversed.find(subs_id[1]) != subs_traversed.end());
   BOOST_CHECK(subs_traversed.find(subs_id[2]) != subs_traversed.end());
}

BOOST_AUTO_TEST_SUITE_END()
