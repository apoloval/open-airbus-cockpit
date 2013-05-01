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

std::pair<Ptr<Buffer>, Ptr<Logger>> InitLogger()
{
   Ptr<Buffer> buff = new FixedBuffer(1024);
   Ptr<Logger> logger = new Logger(
         LogLevel::INFO, new BufferOutputStream(buff));
   return std::pair<Ptr<Buffer>, Ptr<Logger>>(buff, logger);
}

BOOST_AUTO_TEST_CASE(ShouldInitLogger)
{
   auto logger = InitLogger();
   Logger::setMain(logger.second);
   BOOST_CHECK_EQUAL(logger.second, Logger::main());
}

BOOST_AUTO_TEST_CASE(ShouldWriteInLogger)
{
   auto logger = InitLogger();
   logger.second->log(LogLevel::INFO, "ABCD");
   Reader reader(new BufferInputStream(logger.first));
   auto line = reader.readLine();
   BOOST_CHECK(line.find("[INFO]") != std::string::npos);
   BOOST_CHECK(line.find("ABCD") != std::string::npos);
}