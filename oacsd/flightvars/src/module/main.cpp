/*
 * This file is part of Open Airbus Cockpit
 * Copyright (C) 2012 Alvaro Polo
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

#include "core.h"
#include "fsuipc.h"

#define LOG_FILE "C:\\Windows\\Temp\\FlightVars.log"

using namespace oac;
using namespace oac::fv;

void __stdcall DLLStart(void)
{
   file log_file(LOG_FILE);
   Logger::setMain(new Logger(LogLevel::INFO, log_file.append()));
   try
   {
      Log(oac::INFO, "flight_vars module is starting");
      auto core = flight_vars_core::instance();
      core->register_group_master(
            fsuipc_flight_vars::VAR_GROUP, new fsuipc_flight_vars());
      Log(oac::INFO, "flight_vars module has been started");
   } catch (error& e)
   {
      Log(oac::FAIL, boost::format("Unexpected error: %s") %
          boost::diagnostic_information(e));
   }
}

void __stdcall DLLStop(void)
{
   Log(oac::INFO, "flight_vars module is stopping");
   // Destroy core here
   Log(oac::INFO, "flight_vars module has been stopped");
   Logger::setMain(nullptr); // close the main logger
}
