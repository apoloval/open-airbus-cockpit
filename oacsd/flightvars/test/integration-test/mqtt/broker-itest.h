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

#include <thread>

#include <liboac/filesystem.h>
#include <liboac/logging.h>

#include "mqtt/broker.h"

using namespace oac;
using namespace oac::fv;
using namespace oac::fv::mqtt;

struct logging_fixture
{
   logging_fixture()
   {
      auto logger = make_logger(log_level::INFO, file_output_stream::STDERR);
      set_main_logger(logger);
   }
};

BOOST_FIXTURE_TEST_SUITE(MosquittoBrokerIT, logging_fixture)

BOOST_AUTO_TEST_CASE(MustRunBrokerAsBackgroundProcess)
{
   mosquitto_process_runner bh;
   bh.run_broker();
   std::this_thread::sleep_for(std::chrono::seconds(5));
   bh.shutdown_broker();
}

BOOST_AUTO_TEST_SUITE_END()
