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

#include "client/requests.h"
#include "subscription.h"

using namespace oac;
using namespace oac::fv;
using namespace oac::fv::client;

auto null_handler = [](const variable_id&, const variable_value&) {};



BOOST_AUTO_TEST_SUITE(ClientRequestTest)

BOOST_AUTO_TEST_CASE(MustSetAndGetResult)
{
   auto var_id = make_var_id("foobar", "datum");
   request_pool pool;
   auto req = std::make_shared<subscription_request>(var_id, null_handler);

   req->set_result(1234);

   BOOST_CHECK_EQUAL(1234, req->get_result(std::chrono::seconds(1)));
}

BOOST_AUTO_TEST_CASE(MustThrowOnGetTimeOut)
{
   auto var_id = make_var_id("foobar", "datum");
   request_pool pool;
   auto req = std::make_shared<subscription_request>(var_id, null_handler);

   BOOST_CHECK_THROW(
         req->get_result(std::chrono::milliseconds(10)),
         client::request_timeout_error);
}

BOOST_AUTO_TEST_CASE(MustThrowOnErrorSet)
{
   auto var_id = make_var_id("foobar", "datum");
   request_pool pool;
   auto req = std::make_shared<subscription_request>(var_id, null_handler);

   req->set_error(
         OAC_MAKE_EXCEPTION(flight_vars::no_such_variable_error(var_id)));

   BOOST_CHECK_THROW(
         req->get_result(std::chrono::seconds(1)),
         flight_vars::no_such_variable_error);
}

BOOST_AUTO_TEST_SUITE_END()



BOOST_AUTO_TEST_SUITE(ClientRequestPoolTest)

BOOST_AUTO_TEST_CASE(MustInsertSubscription)
{
   auto var_id = make_var_id("foobar", "datum");
   request_pool pool;
   auto req = std::make_shared<subscription_request>(var_id, null_handler);

   pool.insert(req);

   auto requests = pool.pop_subscription_requests(var_id);
   BOOST_REQUIRE_EQUAL(1, requests.size());
   BOOST_CHECK_EQUAL(var_id, requests.front()->var_id());
}

BOOST_AUTO_TEST_CASE(MustPopEmptyListOnMissingSubscriptions)
{
   auto var_id = make_var_id("foobar", "datum");
   request_pool pool;

   auto requests = pool.pop_subscription_requests(var_id);
   BOOST_CHECK_EQUAL(0, requests.size());
}

BOOST_AUTO_TEST_CASE(MustPopEmptyListAfterPopSubscriptions)
{
   auto var_id = make_var_id("foobar", "datum");
   request_pool pool;
   auto req = std::make_shared<subscription_request>(var_id, null_handler);

   pool.insert(req);

   auto requests = pool.pop_subscription_requests(var_id);
   requests = pool.pop_subscription_requests(var_id);
   BOOST_CHECK_EQUAL(0, requests.size());
}

BOOST_AUTO_TEST_CASE(MustInsertUnsubscription)
{
   auto master_subs_id = make_subscription_id();
   auto slave_subs_id = make_subscription_id();
   request_pool pool;
   auto req = std::make_shared<unsubscription_request>(slave_subs_id);
   req->update_master_subs_id(master_subs_id);

   pool.insert(req);

   auto requests = pool.pop_unsubscription_requests(master_subs_id);
   BOOST_REQUIRE_EQUAL(1, requests.size());
   BOOST_CHECK_EQUAL(master_subs_id, requests.front()->master_subs_id());
   BOOST_CHECK_EQUAL(slave_subs_id, requests.front()->virtual_subs_id());
}

BOOST_AUTO_TEST_CASE(MustPopEmptyListOnMissingUnsubscriptions)
{
   auto master_subs_id = make_subscription_id();
   request_pool pool;

   auto requests = pool.pop_unsubscription_requests(master_subs_id);
   BOOST_CHECK_EQUAL(0, requests.size());
}

BOOST_AUTO_TEST_CASE(MustPopEmptyListAfterPopUnsubscriptions)
{
   auto master_subs_id = make_subscription_id();
   auto slave_subs_id = make_subscription_id();
   request_pool pool;
   auto req = std::make_shared<unsubscription_request>(slave_subs_id);
   req->update_master_subs_id(master_subs_id);

   pool.insert(req);

   auto requests = pool.pop_unsubscription_requests(master_subs_id);
   requests = pool.pop_unsubscription_requests(master_subs_id);
   BOOST_CHECK_EQUAL(0, requests.size());
}

BOOST_AUTO_TEST_CASE(MustPropagateErrors)
{
   auto var_id = make_var_id("foobar", "datum");
   request_pool pool;
   auto req1 = std::make_shared<subscription_request>(var_id, null_handler);
   auto req2 = std::make_shared<subscription_request>(var_id, null_handler);

   pool.insert(req1);
   pool.insert(req2);

   pool.propagate_error(
         OAC_MAKE_EXCEPTION(flight_vars::no_such_variable_error(var_id)));
   BOOST_CHECK_THROW(
         req1->get_result(std::chrono::seconds(1)),
         flight_vars::no_such_variable_error);
   BOOST_CHECK_THROW(
         req2->get_result(std::chrono::seconds(1)),
         flight_vars::no_such_variable_error);
   BOOST_CHECK_EQUAL(
         0,
         pool.pop_subscription_requests(var_id).size());
}

BOOST_AUTO_TEST_SUITE_END()
