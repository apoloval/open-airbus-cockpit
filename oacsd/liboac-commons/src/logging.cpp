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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Open Airbus Cockpit.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <cstdio>
#include <ctime>

#include "logging.h"

namespace oac {

namespace {

const char* LEVEL_STR[] =
{
   "INFO", "WARN", "FAIL"
};

ptr<abstract_logger> main_logger;

} // anonymous namespace

const char*
abstract_logger::level_str(log_level level)
{ return LEVEL_STR[level]; }

std::string
abstract_logger::get_time()
{
   auto t = time(nullptr);
   char time_buf[26];
   struct tm lt;
   localtime_s(&lt, &t);
   asctime_s(time_buf, &lt);
   time_buf[24] = '\0';
   return time_buf;
}

void
set_main_logger(const ptr<abstract_logger>& logger)
{
   main_logger = logger;
}

ptr<abstract_logger>
get_main_logger()
{ return main_logger; }


void
log(log_level level, const std::string& msg)
{
   auto main = get_main_logger();
   if (main)
      main->log(level, msg);
}

void
log(log_level level, const boost::format& fmt)
{ log(level, str(fmt)); }

} // namespace oac
