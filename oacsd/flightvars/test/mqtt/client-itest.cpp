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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You Must have received a copy of the GNU General Public License
 * along with Open Airbus Cockpit. If not, see <http://www.gnu.org/licenses/>.
 */

#define BOOST_AUTO_TEST_MAIN
#include <boost/test/auto_unit_test.hpp>

#include <thread>
#include <queue>

#include <liboac/filesystem.h>
#include <liboac/logging.h>
#include <liboac/thread/monitor.h>

#include "mqtt/broker.h"
#include "mqtt/client_mosquitto.h"

using namespace oac;
using namespace oac::fv;

BOOST_AUTO_TEST_SUITE(MqttMosquittoClientIT)

struct let_test
{
   let_test() : _broker_running(false)
   {
      auto logger = make_logger(log_level::TRACE, file_output_stream::STDERR);
      set_main_logger(logger);
   }

   ~let_test()
   {
      if (_broker_running)
      {
         _broker_runner.shutdown_broker();
      }
   }

   let_test& given_broker()
   {
      _broker_runner.run_broker();
      _broker_running = true;
      return *this;
   }

   let_test& connect()
   {
      try
      { _cli.reset(new mqtt::mosquitto_client()); }
      catch (...)
      { _error = std::current_exception(); }
      return *this;
   }

   let_test& disconnect()
   {
      try
      { _cli.reset(); }
      catch (...)
      { _error = std::current_exception(); }
      return *this;
   }

   let_test& connect_probe()
   {
      _probe.reset(new mqtt::mosquitto_client());
      return *this;
   }

   let_test& disconnect_probe()
   {
      try
      { _probe.reset(); }
      catch (...)
      { _error = std::current_exception(); }
      return *this;
   }

   let_test& subscribe_probe(const mqtt::topic_pattern& pattern)
   {
      _probe->subscribe_as<int>(
            pattern,
            mqtt::qos_level::LEVEL_0,
            [this](const int& number)
      {
         _received_messages->push(number);
      });
      return *this;
   }

   let_test& publish(
         const mqtt::topic& t,
         int num)
   {
      _cli->publish_as<int>(t, num);
      return *this;
   }

   let_test& publish_burst(
         const mqtt::topic& t,
         int from,
         int to)
   {
      for (int i = from; i <= to; i++)
         _cli->publish_as<int>(t, i);
      return *this;
   }

   template <typename Error>
   let_test& assert_error()
   {
      BOOST_CHECK_THROW(
         {
            if (_error != std::exception_ptr())
               std::rethrow_exception(_error);
         },
         Error
      );
      return *this;
   }

   let_test& assert_no_error()
   {
      BOOST_CHECK(std::exception_ptr() == _error);
      return *this;
   }

   let_test& assert_published(
         const mqtt::topic& t,
         int num)
   {
      sleep(2500);
      BOOST_CHECK_EQUAL(num, _received_messages->back());
      return *this;
   }

   let_test& assert_burst_published(
         const mqtt::topic& t,
         int from,
         int to)
   {
      sleep(2500);
      for (int i = from; i <= to; i++)
      {
         BOOST_CHECK_EQUAL(i, _received_messages->front());
         _received_messages->pop();
      }
      return *this;
   }

private:

   std::unique_ptr<mqtt::mosquitto_client> _cli;
   std::unique_ptr<mqtt::mosquitto_client> _probe;
   mqtt::mosquitto_process_runner _broker_runner;
   bool _broker_running;
   std::exception_ptr _error;
   thread::monitor<std::queue<int>> _received_messages;

   void sleep(int millis)
   {
      std::this_thread::sleep_for(std::chrono::milliseconds(millis));
   }
};

BOOST_AUTO_TEST_CASE(MustConnectAndDisconnectToBroker)
{
   let_test()
         .given_broker()
         .connect()
         .disconnect()
         .assert_no_error();
}

BOOST_AUTO_TEST_CASE(MustThrowOnConnectionError)
{
   let_test()
         .connect()
         .assert_error<mqtt::connection_error>();
}

BOOST_AUTO_TEST_CASE(MustPublishSubscribe)
{
   let_test()
         .given_broker()
         .connect()
         .connect_probe()
         .subscribe_probe("foo/+")
         .publish("foo/bar", 1234)
         .assert_published("/foo/bar", 1234)
         .disconnect()
         .disconnect_probe();
}

BOOST_AUTO_TEST_CASE(MustPublishInBurst)
{
   let_test()
         .given_broker()
         .connect()
         .connect_probe()
         .subscribe_probe("foo/+")
         .publish_burst("foo/bar", 1, 1000)
         .assert_burst_published("/foo/bar", 1, 1000)
         .disconnect()
         .disconnect_probe();
}

BOOST_AUTO_TEST_SUITE_END()
