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

#include <iostream>

#include <boost/property_tree/json_parser.hpp>

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

   template <typename... T>
   let_test& with_array_item(
         const std::string& prop,
         T... t)
   {
      if (!pt.get_child_optional(prop))
      {
         ptree child;
         pt.put_child(prop, child);
      }
      ptree array = pt.get_child(prop);
      ptree item;
      fill_array(item, t...);
      array.push_back(std::make_pair("", item));
      pt.put_child(prop, array);
      return *this;
   }

   let_test& print_json()
   {
      boost::property_tree::write_json("C:\\Windows\\Temp\\test.json", pt);
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

   template <typename T1, typename... T>
   void fill_array(
         ptree& pt,
         const std::string& prop,
         const T1& value,
         T... other)
   {
      pt.put(prop, value);
      fill_array(pt, other...);
   }

   template <typename T>
   void fill_array(
         ptree& pt,
         const std::string& prop,
         const T& value)
   {
      pt.put(prop, value);
   }
};

#define GET_SETTING(prop) \
      [](const flightvars_settings& s) { return s.prop; }

BOOST_AUTO_TEST_CASE(MustReadLoggingEnabled)
{
   let_test()
      .with_input("logging.enabled", true)
      .load_settings()
      .assert_equal(true, GET_SETTING(logging.enabled));
}

BOOST_AUTO_TEST_CASE(MustReadDefaultLoggingEnabled)
{
   let_test()
      .load_settings()
      .assert_equal(true, GET_SETTING(logging.enabled));
}

BOOST_AUTO_TEST_CASE(MustReadLoggingFile)
{
   let_test()
      .with_input("logging.file", "C:\\Path\\To\\Log\\File.log")
      .load_settings()
      .assert_equal(
            "C:\\Path\\To\\Log\\File.log",
            GET_SETTING(logging.file));
}

BOOST_AUTO_TEST_CASE(MustReadDefaultLoggingFile)
{
   let_test()
      .load_settings()
      .assert_equal(
            flightvars_settings::DEFAULT_LOG_FILE,
            GET_SETTING(logging.file));
}

BOOST_AUTO_TEST_CASE(MustReadLoggingLevel)
{
   let_test()
      .with_input("logging.level", "error")
      .load_settings()
      .assert_equal(log_level::ERROR, GET_SETTING(logging.level));
}

BOOST_AUTO_TEST_CASE(MustThrowOnInvalidLoggingLevel)
{
   let_test()
      .with_input("logging.level", "that-unexisting-level")
      .load_settings()
      .assert_error<conf::config_exception>();
}

BOOST_AUTO_TEST_CASE(MustReadDefaultLoggingLevel)
{
   let_test()
      .load_settings()
      .assert_equal(log_level::WARN, GET_SETTING(logging.level));
}

BOOST_AUTO_TEST_CASE(MustReadMqttBrokerRunner)
{
   let_test()
      .with_input("mqtt.broker.runner", "mosquitto-process")
      .load_settings()
      .assert_equal(
            mqtt_broker_runner_id::MOSQUITTO_PROCESS,
            GET_SETTING(mqtt.broker.runner));
}

BOOST_AUTO_TEST_CASE(MustThrowOnInvalidMqttBrokerRunner)
{
   let_test()
      .with_input("mqtt.broker.runner", "no-such-runner")
      .load_settings()
      .assert_error<conf::config_exception>();
}

BOOST_AUTO_TEST_CASE(MustReadDefaulMqttBrokerRunner)
{
   let_test()
      .load_settings()
      .assert_equal(
            flightvars_settings::DEFAULT_MQTT_BROKER_RUNNER,
            GET_SETTING(mqtt.broker.runner));
}

BOOST_AUTO_TEST_CASE(MustReadDomainProperties)
{
   let_test()
      .with_array_item("domains",
            "name", "fsuipc-offsets",
            "description", "Access to FSUIPC offsets",
            "customField1", 1000,
            "customField2", true)
      .with_array_item("domains",
            "name", "dom2",
            "description", "The Domain #2",
            "enabled", false,
            "exports", "C:\\ProgramData\\OACSD\\Exports-Custom2.json",
            "customField3", 3.14,
            "customField4", "barbecue")
      .load_settings()
      .assert_equal(conf::domain_type::FSUIPC_OFFSETS,
            GET_SETTING(domains[0].type))
      .assert_equal("fsuipc-offsets",
            GET_SETTING(domains[0].name))
      .assert_equal(
            "Access to FSUIPC offsets",
            GET_SETTING(domains[0].description))
      .assert_equal(
            true,
            GET_SETTING(domains[0].enabled))
      .assert_equal(
            conf::domain_settings::DEFAULT_FSUIPC_OFFSETS_EXPORT_FILE,
            GET_SETTING(domains[0].exports_file))
      .assert_equal(
            1000,
            GET_SETTING(domains[0].properties.get<int>("customField1")))
      .assert_equal(
            true,
            GET_SETTING(domains[0].properties.get<bool>("customField2")))
      .assert_equal(conf::domain_type::CUSTOM,
            GET_SETTING(domains[1].type))
      .assert_equal(
            "dom2",
            GET_SETTING(domains[1].name))
      .assert_equal(
            "The Domain #2",
            GET_SETTING(domains[1].description))
      .assert_equal(
            false,
            GET_SETTING(domains[1].enabled))
      .assert_equal(
            boost::filesystem::path(
                  "C:\\ProgramData\\OACSD\\Exports-Custom2.json"),
            GET_SETTING(domains[1].exports_file))
      .assert_equal(
            3.14,
            GET_SETTING(domains[1].properties.get<double>("customField3")))
      .assert_equal(
            "barbecue",
            GET_SETTING(domains[1].properties.get<std::string>("customField4")));
}

BOOST_AUTO_TEST_SUITE_END()
