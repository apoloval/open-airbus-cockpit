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

#include <SimConnect.h>

#include <liboac/cockpit-fsuipc.h>
#include <liboac/fsuipc.h>
#include <liboac/logging.h>

#include "wilco.h"

#define LOG_FILE "C:\\Windows\\Temp\\WilcoExporter.log"
#define AIRCRAFT_TYPE A320_CFM

using namespace oac;
using namespace oac::we;

namespace {

enum EVENT_ID
{
	EVENT_1SEC_ELAPSED,
};

WilcoCockpit* InitWilcoCockpit()
{
   Log(INFO, "Initializing Wilco Cockpit module... ");
   auto result = WilcoCockpit::newCockpit(AIRCRAFT_TYPE);
   Log(INFO, "Wilco Cockpit module successfully initialized");
   return result;
}

void Print(const WilcoCockpit& wilco)
{
   wilco.debug();
}

void CALLBACK DispatchProc(SIMCONNECT_RECV* pData, DWORD cbData, void* pContext)
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

HANDLE handleSimConnect;

void Connect()
{
	HRESULT res;

	res = SimConnect_Open(&handleSimConnect, "WilcoExporter", NULL, 0, 0, 0);
	if (res != S_OK)
		Log(FAIL, "Cannot open connection to SimConnect");

	res = SimConnect_CallDispatch(handleSimConnect, DispatchProc, NULL);
	if (res != S_OK)
		Log(FAIL, "Cannot register SimConnect callback");

	res = SimConnect_SubscribeToSystemEvent(handleSimConnect, EVENT_1SEC_ELAPSED, "1sec");
	if (res == S_OK)
		Log(INFO, "Subscription to SimConnect 1sec system event done successfully");
	else
		Log(FAIL, "The callback subscription to SimConnect 1sec system event was failed!!!");
}

void Disconnect()
{
   auto res = SimConnect_Close(handleSimConnect);
	if (res != S_OK)
		Log(FAIL, "Cannot close connection to SimConnect");
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