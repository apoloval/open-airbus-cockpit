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

#include <Windows.h>

#include <liboac/filesystem.h>
#include <liboac/logging.h>

#include "mqtt/broker.h"

#define LOG_FILE "C:\\Windows\\Temp\\FlightVars.log"

using namespace oac;
using namespace oac::fv;

std::unique_ptr<mqtt::broker_runner> bh;

void __stdcall DLLStart(void)
{
   try
   {
      file log_file(LOG_FILE);
      set_main_logger(make_logger(log_level::INFO, log_file.append()));

      bh.reset(new mqtt::mosquitto_process_runner());
      bh->run_broker();
   }
   catch (oac::exception& e)
   {
      log(
            "DLLStart",
            log_level::FATAL,
            format("Unexpected error: %s", e.report()));
   }
   catch (...)
   {
      log(
            "DLLStart",
            log_level::FATAL,
            "Unexpected error: unknown exception thrown");
   }
}

void __stdcall DLLStop(void)
{
   try
   {
      bh->shutdown_broker();
      bh.reset();
   }
   catch (oac::exception& e)
   {
      log(
            "DLLStop",
            log_level::FATAL,
            format("Unexpected error: %s", e.report()));
   }
   catch (...)
   {
      log(
            "DLLStop",
            log_level::FATAL,
            "Unexpected error: unknown exception thrown");
   }
}
