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

#include <boost/test/auto_unit_test.hpp>

#include "conf/provider.h"

using namespace oac;
using namespace oac::fv;
using namespace oac::fv::conf;
using namespace boost::property_tree;

BOOST_AUTO_TEST_SUITE(BtpLoadSettingsTest)

void init_tree(ptree& pt)
{
   pt.put("logging.enabled", true);
}

struct let_test
{
   template <typename T>
   let_test& with_input(
         const std::string& prop,
         const T& value)
   {
      pt.put(prop, value);
      return *this;
   }

   let_test& load_settings()
   {
      try
      {
         bpt_load_settings(pt, settings);
      } catch (...)
      {
         error = std::current_exception();
      }
      return *this;
   }

   template <typename T, typename Accessor>
   let_test& assert_equal(const T& value, Accessor access)
   {
      BOOST_CHECK(value == access(settings));
      return *this;
   }

   template <typename Error>
   let_test& assert_error()
   {
      BOOST_CHECK_THROW(
         {
            if (error != std::exception_ptr())
               std::rethrow_exception(error);
         },
         Error
      );
      return *this;
   }

private:

   ptree pt;
   flightvars_settings settings;
   std::exception_ptr error;
};

BOOST_AUTO_TEST_CASE(MustReadLoggingEnabled)
{
   let_test()
      .with_input("logging.enabled", true)
      .load_settings()
      .assert_equal(
            true,
            [](const flightvars_settings& s) { return s.logging.enabled; });
}

BOOST_AUTO_TEST_CASE(MustReadDefaultLoggingEnabled)
{
   let_test()
      .load_settings()
      .assert_equal(
            true,
            [](const flightvars_settings& s) { return s.logging.enabled; });
}

BOOST_AUTO_TEST_CASE(MustReadLoggingFile)
{
   let_test()
      .with_input("logging.file", "C:\\Path\\To\\Log\\File.log")
      .load_settings()
      .assert_equal(
            "C:\\Path\\To\\Log\\File.log",
            [](const flightvars_settings& s) { return s.logging.file; });
}

BOOST_AUTO_TEST_CASE(MustReadDefaultLoggingFile)
{
   let_test()
      .load_settings()
      .assert_equal(
            flightvars_settings::default_log_file(),
            [](const flightvars_settings& s) { return s.logging.file; });
}

BOOST_AUTO_TEST_CASE(MustReadLoggingLevel)
{
   let_test()
      .with_input("logging.level", "error")
      .load_settings()
      .assert_equal(
            log_level::ERROR,
            [](const flightvars_settings& s) { return s.logging.level; });
}

BOOST_AUTO_TEST_CASE(MustThrowOnInvalidLoggingLevel)
{
   let_test()
      .with_input("logging.level", "that-unexisting-level")
      .load_settings()
      .assert_error<invalid_config_error>();
}

BOOST_AUTO_TEST_CASE(MustReadDefaultLoggingLevel)
{
   let_test()
      .load_settings()
      .assert_equal(
            log_level::WARN,
            [](const flightvars_settings& s) { return s.logging.level; });
}

BOOST_AUTO_TEST_CASE(MustReadMqttBrokerRunner)
{
   let_test()
      .with_input("mqtt.broker.runner", "mosquitto-process")
      .load_settings()
      .assert_equal(
            mqtt_broker_runner_id::MOSQUITTO_PROCESS,
            [](const flightvars_settings& s) { return s.mqtt.broker.runner; });
}

BOOST_AUTO_TEST_CASE(MustThrowOnInvalidMqttBrokerRunner)
{
   let_test()
      .with_input("mqtt.broker.runner", "no-such-runner")
      .load_settings()
      .assert_error<invalid_config_error>();
}

BOOST_AUTO_TEST_CASE(MustReadDefaulMqttBrokerRunner)
{
   let_test()
      .load_settings()
      .assert_equal(
            flightvars_settings::DEFAULT_MQTT_BROKER_RUNNER,
            [](const flightvars_settings& s) { return s.mqtt.broker.runner; });
}

BOOST_AUTO_TEST_SUITE_END()
