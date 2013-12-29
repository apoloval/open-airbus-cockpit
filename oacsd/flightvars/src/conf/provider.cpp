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
      OAC_THROW_EXCEPTION(illegal_property_value(prop, text, e));
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
bpt_load_domain_type(
      const boost::property_tree::ptree& props,
      domain_type& type)
{
   auto domain_name = props.get<std::string>("name");
   try
   { type = domain_type_conversions::from_string(domain_name); }
   catch (const util::enum_tag_error&)
   { type = domain_type::CUSTOM; }
}

void
bpt_load_exports_file(
      const boost::property_tree::ptree& props,
      boost::filesystem::path& exports)
{
   domain_type type;
   bpt_load_domain_type(props, type);
   boost::filesystem::path default_exports;
   switch (type)
   {
      case domain_type::FSUIPC_OFFSETS:
         default_exports = domain_settings::DEFAULT_FSUIPC_OFFSETS_EXPORT_FILE;
         break;
      default:
         auto name = props.get<std::string>("name");
         default_exports = boost::filesystem::path("C:\\ProgramData\\OACSD\\") /
               format("Exports-%s.json", name);
         break;
   }
   bpt_load_path(props, "exports", default_exports, exports);
}

void
bpt_load_domain_settings(
      const boost::property_tree::ptree& domain_props,
      std::vector<domain_settings>& domains)
{
   for (auto& dom : domain_props)
   {
      auto& props = dom.second;
      domain_settings settings;

      settings.properties = props;
      settings.name = props.get("name", "");
      settings.description = props.get("description", "");
      settings.enabled = props.get("enabled", true);
      bpt_load_domain_type(props, settings.type);
      bpt_load_exports_file(props,settings.exports_file);

      domains.push_back(settings);
   }
}

} // anonymous namespace

void
bpt_load_settings(
      const boost::property_tree::ptree& props,
      flightvars_settings& settings)
throw (config_exception)
{
   settings.logging.enabled = props.get(
         "logging.enabled",
         true);
   bpt_load_path(
         props,
         "logging.file",
         flightvars_settings::DEFAULT_LOG_FILE,
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
   bpt_load_enum_setting<
         mqtt_client_id, mqtt_client_id_conversions>(
         props,
         "mqtt.client",
         flightvars_settings::DEFAULT_MQTT_CLIENT,
         settings.mqtt.client);
   auto dom_props = props.get_child_optional("domains");
   if (dom_props)
      bpt_load_domain_settings(*dom_props, settings.domains);
}

void
bpt_json_config_loader::load_from_file(
      const boost::filesystem::path& file,
      boost::property_tree::ptree& pt)
{
   try
   { boost::property_tree::read_json(file.string(), pt); }
   catch (const boost::property_tree::json_parser_error& e)
   { OAC_THROW_EXCEPTION(invalid_config_format(file, "JSON", e)); }
}

void
bpt_xml_config_loader::load_from_file(
      const boost::filesystem::path& file,
      boost::property_tree::ptree& pt)
{
   try { boost::property_tree::read_xml(file.string(), pt); }
   catch (const boost::property_tree::xml_parser_error& e)
   { OAC_THROW_EXCEPTION(invalid_config_format(file, "XML", e)); }
}

void
bpt_auto_config_loader::load_from_file(
      const boost::filesystem::path& file,
      boost::property_tree::ptree& pt)
{
   auto ext = file.extension();
   if (ext == ".json")
      return bpt_json_config_loader().load_from_file(file, pt);
   if (ext == ".xml")
      return bpt_xml_config_loader().load_from_file(file, pt);

   // Unknown extension: let's try with the rest of loaders until one succeeds.
   try { return bpt_json_config_loader().load_from_file(file, pt); }
   catch (...) {}
   try { return bpt_xml_config_loader().load_from_file(file, pt); }
   catch (...) {}

   // No loader succeed.
   OAC_THROW_EXCEPTION(invalid_config_format(file, "any known"));
}

void
bpt_config_provider::load_settings(
      flightvars_settings& settings)
throw (config_exception)
{
   using boost::filesystem::path;
   using boost::filesystem::exists;

   auto json_file = path(_config_dir) / "FlightVars.json";
   auto xml_file = path(_config_dir) / "FlightVars.xml";

   boost::property_tree::ptree pt;
   if (exists(json_file))
      bpt_json_config_loader().load_from_file(json_file, pt);
   else if (exists(xml_file))
      bpt_xml_config_loader().load_from_file(xml_file, pt);

   // Regardless it was loaded or not, read settings from pt; if the tree was
   // not loaded, the default settings will be loaded.
   bpt_load_settings(pt, settings);
}


}}} // namespace oac::fv::conf
