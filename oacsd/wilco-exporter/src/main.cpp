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

#include <sstream>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <liboac/cockpit-fsuipc.h>
#include <liboac/filesystem.h>
#include <liboac/fsuipc.h>
#include <liboac/logging.h>
#include <liboac/simconn.h>

#include "plugin.h"

#define LOG_FILE "C:\\Windows\\Temp\\WilcoExporter.log"

using namespace oac;
using namespace oac::we;

plugin_ptr plug;

void __stdcall DLLStart(void)
{
   file log_file(LOG_FILE);
   set_main_logger(make_logger(log_level::INFO, log_file.append()));
   log("DLLStart", log_level::INFO, "The Wilco Exporter module is starting");
   plug = std::make_shared<plugin>();
   log("DLLStart", log_level::INFO, "The Wilco Exporter module has been started");
}

void __stdcall DLLStop(void)
{
   log("DLLStop", log_level::INFO, "The Wilco Exporter module is stopping");
   plug.reset();
   log("DLLStop", log_level::INFO, "The Wilco Exporter module has been stopped");
   close_main_logger(); // close the main logger
}
