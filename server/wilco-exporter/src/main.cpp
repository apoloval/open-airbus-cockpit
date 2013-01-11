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

#include "wilco.h"

#define LOG_FILE "C:\\Windows\\Temp\\WilcoExporter.log"
#define AIRCRAFT_TYPE A320_CFM

using namespace oac;
using namespace oac::we;

namespace {

WilcoCockpit* InitWilcoCockpit()
{
   Log(INFO, "Initializing Wilco Cockpit module... ");
   auto result = WilcoCockpit::newCockpit(AIRCRAFT_TYPE);
   Log(INFO, "Wilco Cockpit module successfully initialized");
   return result;
}

void OnEvent(SimConnectClient& sc, const SIMCONNECT_RECV_EVENT& msg)
{
   try 
   {
      static WilcoCockpit* wilco = InitWilcoCockpit();
      static CockpitBack* back = new FSUIPCCockpitBack(
            new LocalFSUIPC<1024>::Factory());

      back->map(*wilco);
   } catch (std::exception& ex) {
      Log(WARN, ex.what());
   }
}

Ptr<SimConnectClient> sc_client;

void Connect()
{   
   sc_client = new SimConnectClient("Wilco Exporter");
   sc_client->registerOnEventCallback(OnEvent);
   sc_client->subscribeToSystemEvent(SimConnectClient::SYSTEM_EVENT_1SEC);
}

void Disconnect()
{
   sc_client.reset();
}

}; // unnamed namespace

void __stdcall DLLStart(void)
{
   InitLogger(LOG_FILE);
	Log(oac::INFO, "The Wilco Exporter module is starting");
	Connect();
	Log(oac::INFO, "The Wilco Exporter module has been started");
}

void __stdcall DLLStop(void)
{
	Log(oac::INFO, "The Wilco Exporter module is stopping");
	Disconnect();
	Log(oac::INFO, "The Wilco Exporter module has been stopped");
   CloseLogger();
}