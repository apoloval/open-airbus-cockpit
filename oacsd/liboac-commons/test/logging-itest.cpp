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

#include "buffer.h"
#include "logging.h"

using namespace oac;

std::pair<ptr<fixed_buffer>, ptr<abstract_logger>> init_logger()
{
   auto buff = make_ptr(new fixed_buffer(1024));
   auto log = make_logger(
         log_level::INFO, buffer::make_output_stream(buff));
   return std::make_pair(buff, log);
}

BOOST_AUTO_TEST_CASE(ShouldInitlogger)
{
   auto log = init_logger();
   set_main_logger(log.second);
   BOOST_CHECK_EQUAL(log.second, get_main_logger());
}

BOOST_AUTO_TEST_CASE(ShouldWriteInlogger)
{
   auto log = init_logger();
   log.second->log(log_level::INFO, "ABCD");
   auto input = buffer::make_input_stream(log.first);
   auto line = stream::read_line(*input);
   BOOST_CHECK(line.find("[INFO]") != std::string::npos);
   BOOST_CHECK(line.find("ABCD") != std::string::npos);
}
