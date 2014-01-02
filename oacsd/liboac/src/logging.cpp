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
#include <chrono>
#include <iomanip>
#include <sstream>

#include <liboac/logging.h>

namespace oac {

namespace {

const char* LEVEL_STR[] =
{
   "TRACE", "INFO", "WARN", "ERROR", "FATAL"
};

std::shared_ptr<logger> main_logger;

} // anonymous namespace

const char*
logger::level_str(log_level level)
{ return LEVEL_STR[static_cast<int>(level)]; }

std::string
logger::get_time()
{
   std::stringstream ss;
   auto now = std::chrono::high_resolution_clock::now();
   auto now_c = std::chrono::high_resolution_clock::to_time_t(now);
   auto now_m = std::chrono::duration_cast<std::chrono::milliseconds>(
      now.time_since_epoch());
   struct tm local_now;
   localtime_s(&local_now, &now_c);

   ss << std::put_time(&local_now, "%Y-%m-%dT%H:%M:%S");
   ss << "." << std::setfill('0') << std::setw(3) <<  now_m.count() % 1000;

   return ss.str();
}

void
set_main_logger(const std::shared_ptr<logger>& logger)
{
   main_logger = logger;
}

std::shared_ptr<logger>
get_main_logger()
{ return main_logger; }



void
logger_component::log(
      const log_author& author,
      log_level level,
      const log_message& msg)
{
   if (_parent)
      _parent->log(author, level, msg);
   else if (auto main = get_main_logger())
      main->log(author, level, msg);
}

void
log(
      const log_author& author,
      log_level level,
      const log_message& msg)
{
   auto main = get_main_logger();
   if (main)
      main->log(author, level, msg);
}

} // namespace oac
