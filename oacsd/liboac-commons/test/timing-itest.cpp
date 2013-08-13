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

#include <string>

#define BOOST_AUTO_TEST_MAIN
#include <boost/test/auto_unit_test.hpp>

#include <boost/thread.hpp>

#include "filesystem.h"
#include "logging.h"
#include "timing.h"

using namespace oac;

struct simconnect_fixture
{
   simconnect_fixture()
   {
      BOOST_WARN(
            "This test suite requires a running instance of SimConnect "
            "(FSX or Prepar3D) in order to work; otherwise you will obtain "
            "false errors in the test cases");
   }
};

BOOST_FIXTURE_TEST_SUITE(SimConnectTickObserver, simconnect_fixture)

BOOST_AUTO_TEST_CASE(MustConnectAndDisconnectFromSimConnect)
{
   BOOST_CHECK_NO_THROW(simconnect_tick_observer());
}

BOOST_AUTO_TEST_CASE(MustAllowMultipleInstancesAtSameTime)
{
   BOOST_CHECK_NO_THROW(
   {
      simconnect_tick_observer obs1;
      simconnect_tick_observer obs2;
   });
}

BOOST_AUTO_TEST_CASE(MustRegisterTickHandlerAndReceiveTickNotifications)
{
   set_main_logger(make_logger(log_level::INFO, file_output_stream::STDERR));

   simconnect_tick_observer obs;
   int pos = 0, neg = 0;

   obs.register_handler([&]() { pos++; });
   obs.register_handler([&]() { neg--; });

   while (pos < 6)
   {
      obs.dispatch();
   }
   BOOST_CHECK_EQUAL(6, pos);
   BOOST_CHECK_EQUAL(-6, neg);
}

BOOST_AUTO_TEST_SUITE_END()
