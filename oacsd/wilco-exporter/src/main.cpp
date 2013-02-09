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

#include <sstream>
#include <Windows.h>

#include <liboac/cockpit-fsuipc.h>
#include <liboac/fsuipc.h>
#include <liboac/logging.h>
#include <liboac/simconn.h>

#include "plugin.h"

#define LOG_FILE "C:\\Windows\\Temp\\WilcoExporter.log"

using namespace oac;
using namespace oac::we;

Ptr<Plugin> plugin;

void __stdcall DLLStart(void)
{
   InitLogger(LOG_FILE);
	Log(oac::INFO, "The Wilco Exporter module is starting");
	plugin = new Plugin();
	Log(oac::INFO, "The Wilco Exporter module has been started");
}

void __stdcall DLLStop(void)
{
	Log(oac::INFO, "The Wilco Exporter module is stopping");
	plugin.reset();
	Log(oac::INFO, "The Wilco Exporter module has been stopped");
   CloseLogger();
}