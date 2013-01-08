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

#include <iostream>

#define BOOST_AUTO_TEST_MAIN
#include <boost/test/auto_unit_test.hpp>

#include "logging.h"
#include "simconn.h"

using namespace oac;

template <typename MsgType>
void ShouldDispatchSystemEvent(
      const SimConnectClient::EventName& event,
      void (SimConnectClient::*registration)(
            const std::function<void(const MsgType&)>&))
{
   SimConnectClient cli("Test Client");
   bool cont = true;
   (cli.*registration)([&cont](const MsgType& msg)
   {
      cont = false;
   });
   std::cerr << "Should dispatch system event '" << event << "'... ";
   cli.subscribeToSystemEvent(event);
   while (cont) 
   {
      cli.dispatchMessage();
   }
   std::cerr << "Done!" << std::endl;
}

BOOST_AUTO_TEST_CASE(ShouldConnect)
{
   SimConnectClient("Test Client");
}

BOOST_AUTO_TEST_CASE(ShouldRegisterOnFrameCallback)
{
   ShouldDispatchSystemEvent<SIMCONNECT_RECV_EVENT_FRAME>(
         SimConnectClient::SYSTEM_EVENT_FRAME,
         &SimConnectClient::registerOnEventFrameCallback);
}

BOOST_AUTO_TEST_CASE(ShouldRegisterOn1secCallback)
{
   ShouldDispatchSystemEvent<SIMCONNECT_RECV_EVENT>(
         SimConnectClient::SYSTEM_EVENT_1SEC,
         &SimConnectClient::registerOnEventCallback);
}

BOOST_AUTO_TEST_CASE(ShouldRegisterOn4secCallback)
{
   ShouldDispatchSystemEvent<SIMCONNECT_RECV_EVENT>(
         SimConnectClient::SYSTEM_EVENT_4SEC,
         &SimConnectClient::registerOnEventCallback);
}

BOOST_AUTO_TEST_CASE(ShouldRegisterOn6hzCallback)
{
   ShouldDispatchSystemEvent<SIMCONNECT_RECV_EVENT>(
         SimConnectClient::SYSTEM_EVENT_6HZ,
         &SimConnectClient::registerOnEventCallback);
}

BOOST_AUTO_TEST_CASE(ShouldPullRequestDataOnUserObject)
{
   SimConnectClient cli("Test Client");
   std::cerr << "Should pull-request data on user object... ";

   bool cont = true;
   cli.registerOnSimObjectDataCallback([&cont](
         const SIMCONNECT_RECV_SIMOBJECT_DATA& msg)
   {
      struct Data { 
         double qnh; 
         double alt;
         double lat; 
         double lon; 
      };
      Data* data = (Data*) &msg.dwData;
      std::cerr << "QNH: " << data->qnh;
      std::cerr << ", Alt: " << data->alt;
      std::cerr << ", Lat: " << data->lat;
      std::cerr << ", Lon: " << data->lon;
      cont = false;
   });

   auto data_def = cli.newDataDefinition()
         .add("KOHLSMAN SETTING MB", "Millibars")
         .add("Plane Altitude", "Feet")
         .add("Plane Latitude", "Degrees")
         .add("Plane Longitude", "Degrees");
   cli.newDataPullRequest(data_def)
         .setPeriod(SIMCONNECT_PERIOD_SECOND)
         .submit();

   while (cont)
      cli.dispatchMessage();

   std::cerr << ", Done!" << std::endl;
}

BOOST_AUTO_TEST_CASE(ShouldPushRequestDataOnUserObject)
{
   SimConnectClient cli("Test Client");
   std::cerr << "Should push-request data on user object... ";

   double alt = 30000.0;
   auto data_def = cli.newDataDefinition()
         .add("Plane Altitude", "feet");
   cli.newDataPushRequest(data_def)
         .setElementSize(sizeof(double))
         .submit(&alt);

   bool cont = true;
   cli.registerOnSimObjectDataCallback([&cont, &alt](
         const SIMCONNECT_RECV_SIMOBJECT_DATA& msg)
   {
      BOOST_CHECK_CLOSE(alt, *((double*) &msg.dwData), 0.1);
      cont = false;
   });

   cli.newDataPullRequest(data_def).submit();

   while (cont)
      cli.dispatchMessage();

   std::cerr << boost::format("FL%03d, Done!") % int(alt / 100) << std::endl;
}
