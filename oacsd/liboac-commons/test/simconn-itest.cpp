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

#define BOOST_AUTO_TEST_MAIN
#include <boost/test/auto_unit_test.hpp>

#include "logging.h"
#include "simconn.h"

using namespace oac;

template <typename MsgType>
void should_dispatch_system_event(
      const simconnect_client::event_name& event,
      void (simconnect_client::*registration)(
            const std::function<void(simconnect_client&, const MsgType&)>&))
{
   simconnect_client cli("Test Client");
   bool cont = true;
   (cli.*registration)([&cont](simconnect_client& client,const MsgType& msg)
   {
      cont = false;
   });
   BOOST_TEST_MESSAGE("Should dispatch system event '" << event << "'... ");
   cli.subscribe_to_system_event(event);
   while (cont) 
   {
      cli.dispatch_message();
   }
   BOOST_TEST_MESSAGE("Done!");
}

BOOST_AUTO_TEST_CASE(ShouldConnect)
{
   simconnect_client("Test Client");
}

BOOST_AUTO_TEST_CASE(ShouldRegisterOnFrameCallback)
{
   should_dispatch_system_event<SIMCONNECT_RECV_EVENT_FRAME>(
         simconnect_client::SYSTEM_EVENT_FRAME,
         &simconnect_client::register_on_event_frame_callback);
}

BOOST_AUTO_TEST_CASE(ShouldRegisterOn1secCallback)
{
   should_dispatch_system_event<SIMCONNECT_RECV_EVENT>(
         simconnect_client::SYSTEM_EVENT_1SEC,
         &simconnect_client::register_on_event_callback);
}

BOOST_AUTO_TEST_CASE(ShouldRegisterOn4secCallback)
{
   should_dispatch_system_event<SIMCONNECT_RECV_EVENT>(
         simconnect_client::SYSTEM_EVENT_4SEC,
         &simconnect_client::register_on_event_callback);
}

BOOST_AUTO_TEST_CASE(ShouldRegisterOn6hzCallback)
{
   should_dispatch_system_event<SIMCONNECT_RECV_EVENT>(
         simconnect_client::SYSTEM_EVENT_6HZ,
         &simconnect_client::register_on_event_callback);
}

BOOST_AUTO_TEST_CASE(ShouldPullRequestDataOnUserObject)
{
   simconnect_client cli("Test Client");
   BOOST_TEST_MESSAGE("Should pull-request data on user object... ");

   bool cont = true;
   cli.register_on_simobject_data_callback([&cont](
         simconnect_client& client, const SIMCONNECT_RECV_SIMOBJECT_DATA& msg)
   {
      struct Data { 
         double qnh; 
         double alt;
         double lat; 
         double lon; 
      };
      Data* data = (Data*) &msg.dwData;
      BOOST_TEST_MESSAGE("   QNH: " << data->qnh);
      BOOST_TEST_MESSAGE("   Alt: " << data->alt);
      BOOST_TEST_MESSAGE("   Lat: " << data->lat);
      BOOST_TEST_MESSAGE("   Lon: " << data->lon);
      cont = false;
   });

   auto data_def = cli.new_data_definition()
         .add("KOHLSMAN SETTING MB", "Millibars")
         .add("Plane Altitude", "feet")
         .add("Plane Latitude", "degrees")
         .add("Plane Longitude", "degrees");
   cli.new_data_pull_request(data_def)
         .set_period(SIMCONNECT_PERIOD_SECOND)
         .submit();

   while (cont)
      cli.dispatch_message();

   BOOST_TEST_MESSAGE("Done!");
}

BOOST_AUTO_TEST_CASE(ShouldPushRequestDataOnUserObject)
{
   simconnect_client cli("Test Client");
   BOOST_TEST_MESSAGE("Should push-request data on user object... ");

   double alt = 30000.0;
   auto data_def = cli.new_data_definition()
         .add("Plane Altitude", "feet");
   cli.new_data_push_request(data_def)
         .set_element_size(sizeof(double))
         .submit(&alt);

   bool cont = true;
   cli.register_on_simobject_data_callback([&cont, &alt](
         simconnect_client& client, const SIMCONNECT_RECV_SIMOBJECT_DATA& msg)
   {
      BOOST_CHECK_CLOSE(alt, *((double*) &msg.dwData), 0.1);
      cont = false;
   });

   cli.new_data_pull_request(data_def).submit();

   while (cont)
      cli.dispatch_message();

   BOOST_TEST_MESSAGE(boost::format("FL%03d, Done!")
                      % int(alt / 100));
}

BOOST_AUTO_TEST_CASE(ShouldTransmitEventOnUserObject)
{
   simconnect_client cli("Test Client");
   BOOST_TEST_MESSAGE("Should transmit event on user object... ");

   double qnh = 1030.5;
   auto event = cli.new_client_event("KOHLSMAN_SET");
   event.transmit(int(qnh * 16)); // *16 is required by the event representation

   bool cont = true;
   cli.register_on_simobject_data_callback([&cont, &qnh](
         simconnect_client& client, const SIMCONNECT_RECV_SIMOBJECT_DATA& msg)
   {
      BOOST_CHECK_CLOSE(qnh, *((double*) &msg.dwData), 0.1);
      cont = false;
   });
   auto data_def = cli.new_data_definition()
         .add("KOHLSMAN SETTING MB", "Millibars");
   cli.new_data_pull_request(data_def).submit();

   BOOST_TEST_MESSAGE(boost::format("QNH %d, Done!") % int(qnh));
}

BOOST_AUTO_TEST_CASE(ShouldWatchSimpleVariable)
{
   BOOST_TEST_MESSAGE("Should watch simple variable... ");
   simconnect_client::variable_watch<double> qnh;
   qnh.get_data_definition().add("KOHLSMAN SETTING MB", "Millibars");
   BOOST_TEST_MESSAGE(boost::format("QNH %d, Done!")
                      % int(qnh.get()));
}

BOOST_AUTO_TEST_CASE(ShouldWriteWatchSimpleVariable)
{
   BOOST_TEST_MESSAGE("Should write on watch simple variable... ");
   simconnect_client::variable_watch<double> alt;
   alt.get_data_definition().add("Plane Altitude", "feet");

   double value = 25000.0;
   alt.set(value);

   BOOST_CHECK_CLOSE(value, alt.get(), 0.1);

   BOOST_TEST_MESSAGE(boost::format("Alt %d, Done!") % int(value));
}

BOOST_AUTO_TEST_CASE(ShouldWatchTwoSimpleVariables)
{
   BOOST_TEST_MESSAGE("Should watch two simple variables... ");
   simconnect_client::variable_watch<double> qnh;
   qnh.get_data_definition().add("KOHLSMAN SETTING MB", "Millibars");
   simconnect_client::variable_watch<double> alt;
   alt.get_data_definition().add("Plane Altitude", "feet");
   BOOST_TEST_MESSAGE(boost::format("QNH %d, Alt %d; Done!")
      % int(qnh.get()) % int(alt.get()));
}

BOOST_AUTO_TEST_CASE(ShouldWatchComplexVariable)
{
   struct Complex {
      double qnh;
      double alt;
   };

   BOOST_TEST_MESSAGE("Should watch complex variable... ");
   simconnect_client::variable_watch<Complex> watch;
   watch.get_data_definition()
         .add("KOHLSMAN SETTING MB", "Millibars")
         .add("Plane Altitude", "feet");
   auto complex = watch.get();
   BOOST_TEST_MESSAGE(boost::format("QNH %d, Alt %d; Done!")
      % int(complex.qnh) % int(complex.alt));
}

BOOST_AUTO_TEST_CASE(ShouldSetVariableViaEventTransmitter)
{
   BOOST_TEST_MESSAGE("Should watch write-by-event variable... ");
   double qnh = 1013.0;
   
   simconnect_client::event_transmitter event("KOHLSMAN_SET");
   event.transmit(int(16*qnh));

   simconnect_client::variable_watch<double> watch;
   watch.get_data_definition()
         .add("KOHLSMAN SETTING MB", "Millibars");
   double actual_qnh = watch.get();
   BOOST_CHECK_CLOSE(qnh, actual_qnh, 0.1);
   BOOST_TEST_MESSAGE(boost::format("QNH %d, Done!") % int(actual_qnh));
}
