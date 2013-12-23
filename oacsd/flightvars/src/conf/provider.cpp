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

#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <liboac/filesystem.h>

#include "conf/provider.h"

namespace oac { namespace fv { namespace conf {

namespace {

template <typename Enum, typename EnumConversions>
void
bpt_load_enum_setting(
   const boost::property_tree::ptree& props,
   const std::string& prop,
   const Enum& default_value,
   Enum& value)
{
   auto text = props.get(
         prop,
         EnumConversions::to_string(default_value));
   try
   {
      value = EnumConversions::from_string(text);
   } catch (util::enum_tag_error& e)
   {
      OAC_THROW_EXCEPTION(invalid_config_error(prop, text, e));
   }
}

void
bpt_load_path(
      const boost::property_tree::ptree& props,
      const std::string& prop,
      const boost::filesystem::path& default_value,
      boost::filesystem::path& value)
{
   value = props.get(prop, default_value.string());
}

void
bpt_load_domain_settings(
      const boost::property_tree::ptree& domain_props,
      std::vector<domain_settings>& domains)
{
   for (auto& dom : domain_props)
   {
      auto& props = dom.second;
      auto& name = props.get("name", "");
      auto& desc = props.get("description", "");
      domains.push_back({ name, desc, props });
   }
}

} // anonymous namespace

void
bpt_load_settings(
      const boost::property_tree::ptree& props,
      flightvars_settings& settings)
throw (invalid_config_error)
{
   settings.logging.enabled = props.get(
         "logging.enabled",
         true);
   bpt_load_path(
         props,
         "logging.file",
         flightvars_settings::default_log_file(),
         settings.logging.file);
   bpt_load_enum_setting<
         log_level, log_level_conversions>(
         props,
         "logging.level",
         flightvars_settings::DEFAULT_LOG_LEVEL,
         settings.logging.level);
   bpt_load_enum_setting<
         mqtt_broker_runner_id, mqtt_broker_runner_id_conversions>(
         props,
         "mqtt.broker.runner",
         flightvars_settings::DEFAULT_MQTT_BROKER_RUNNER,
         settings.mqtt.broker.runner);
   auto dom_props = props.get_child_optional("domains");
   if (dom_props)
      bpt_load_domain_settings(*dom_props, settings.domains);
}

void
bpt_json_config_provider::load_settings(
      flightvars_settings& settings)
throw (invalid_config_error)
{
   boost::property_tree::ptree pt;
   boost::property_tree::read_json(_config_file.string(), pt);

   bpt_load_settings(pt, settings);
}

void
bpt_xml_config_provider::load_settings(
      flightvars_settings& settings)
throw (invalid_config_error)
{
   boost::property_tree::ptree pt;
   boost::property_tree::read_xml(_config_file.string(), pt);

   bpt_load_settings(pt, settings);
}

void
bpt_config_provider::load_settings(
      flightvars_settings& settings)
throw (invalid_config_error)
{
   using boost::filesystem::path;
   using boost::filesystem::exists;

   auto json_file = path(_config_dir) / "FlightVars.json";
   auto xml_file = path(_config_dir) / "FlightVars.xml";

   if (exists(json_file))
      bpt_json_config_provider(json_file).load_settings(settings);
   else if (exists(xml_file))
      bpt_xml_config_provider(xml_file).load_settings(settings);
   else
   {
      boost::property_tree::ptree pt;
      bpt_load_settings(pt, settings);
   }
}


}}} // namespace oac::fv::conf
