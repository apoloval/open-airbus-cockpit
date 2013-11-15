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

#include <boost/filesystem.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include "conf/provider.h"

using namespace oac;
using namespace oac::fv;
using namespace oac::fv::conf;

BOOST_AUTO_TEST_SUITE(ConfigProviderIT)

struct let_test
{
private:

   boost::filesystem::path _config_dir, _json_file, _xml_file;
   flightvars_settings _settings;

   void set_config(
         boost::property_tree::ptree& pt)
   {
   }

   template <typename... Props>
   void set_config(
         boost::property_tree::ptree& pt,
         const std::string& prop,
         const std::string& value,
         Props... props)
   {
      pt.put(prop, value);
      set_config(pt, props...);
   }

public:

   let_test()
   {
      _config_dir = boost::filesystem::path("C:\\") / "Windows" / "Temp";
      _json_file = _config_dir / "FlightVars.json";
      _xml_file = _config_dir / "FlightVars.xml";

      boost::filesystem::remove(_json_file);
      boost::filesystem::remove(_xml_file);
   }

   template <typename... Props>
   let_test& given_json_config(
         Props... props)
   {
      boost::property_tree::ptree pt;
      set_config(pt, props...);
      boost::property_tree::write_json(_json_file.string(), pt);
      return *this;
   }

   template <typename... Props>
   let_test& given_xml_config(
         Props... props)
   {
      boost::property_tree::ptree pt;
      set_config(pt, props...);
      boost::property_tree::write_xml(_xml_file.string(), pt);
      return *this;
   }

   let_test& load_config()
   {
      bpt_config_provider prov(_config_dir);
      prov.load_settings(_settings);
      return *this;
   }

   template <typename T, typename Accessor>
   let_test& assert_equal(const T& value, Accessor access)
   {
      BOOST_CHECK(value == access(_settings));
      return *this;
   }

};

BOOST_AUTO_TEST_CASE(MustReadConfigFromJsonIfAvailable)
{
   let_test()
         .given_json_config(
               "logging.enabled", "true",
               "mqtt.broker.runner", "mosquitto-process")
         .load_config()
         .assert_equal(
               true,
               [](const flightvars_settings& s) { return s.logging.enabled; })
         .assert_equal(
               mqtt_broker_runner_id::MOSQUITTO_PROCESS,
               [](const flightvars_settings& s) { return s.mqtt.broker.runner; });
}

BOOST_AUTO_TEST_CASE(MustReadConfigFromJsonIfJsonAndXmlAreAvailable)
{
   let_test()
         .given_json_config(
               "logging.enabled", "true",
               "mqtt.broker.runner", "mosquitto-process")
         .given_xml_config(
               "logging.enabled", "false",
               "mqtt.broker.runner", "mosquitto-service")
         .load_config()
         .assert_equal(
               true,
               [](const flightvars_settings& s) { return s.logging.enabled; })
         .assert_equal(
               mqtt_broker_runner_id::MOSQUITTO_PROCESS,
               [](const flightvars_settings& s) { return s.mqtt.broker.runner; });
}

BOOST_AUTO_TEST_CASE(MustReadConfigFromXmlIfJsonIsNotPresent)
{
   let_test()
         .given_xml_config(
               "logging.enabled", "false",
               "mqtt.broker.runner", "mosquitto-service")
         .load_config()
         .assert_equal(
               false,
               [](const flightvars_settings& s) { return s.logging.enabled; })
         .assert_equal(
               mqtt_broker_runner_id::MOSQUITTO_SERVICE,
               [](const flightvars_settings& s) { return s.mqtt.broker.runner; });
}

BOOST_AUTO_TEST_CASE(MustReadDefaultConfigIfJsonAndXmlAreNotPresent)
{
   let_test()
         .load_config()
         .assert_equal(
               true,
               [](const flightvars_settings& s) { return s.logging.enabled; })
         .assert_equal(
               mqtt_broker_runner_id::MOSQUITTO_PROCESS,
               [](const flightvars_settings& s) { return s.mqtt.broker.runner; });
}

BOOST_AUTO_TEST_SUITE_END()
