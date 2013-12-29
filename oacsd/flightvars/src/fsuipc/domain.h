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

#ifndef OAC_FV_FLIGHTVARS_FSUIPC_DOMAIN_H
#define OAC_FV_FLIGHTVARS_FSUIPC_DOMAIN_H

#include <boost/lexical_cast.hpp>
#include <liboac/fsuipc/update_observer.h>
#include <liboac/logging.h>

#include "conf/settings.h"
#include "core/domain.h"

namespace oac { namespace fv { namespace fsuipc {

extern const std::string FSUIPC_OFFSETS_TOPIC_PREFIX;

template <typename FsuipcUserAdapter>
class domain : public core::domain_controller,
               public logger_component
{
public:

   using fsuipc_client_type =
         oac::fsuipc::fsuipc_client<FsuipcUserAdapter>;
   using fsuipc_client_ptr_type =
         oac::fsuipc::fsuipc_client_ptr<FsuipcUserAdapter>;
   using fsuipc_update_observer_type =
         oac::fsuipc::update_observer<FsuipcUserAdapter>;

   domain(
         const conf::domain_settings& settings,
         const mqtt::client_ptr& mqtt_client,
         const fsuipc_client_ptr_type& fsuipc_client)
   throw (core::domain_init_exception)
    : logger_component { "fsuipc_domain" },
      core::domain_controller { mqtt_client },
      _settings { settings },
      _update_obs
      {
         fsuipc_client,
         std::bind(&domain::on_offset_changed, this, std::placeholders::_1)
      }
   {
      load_exports();
      mqtt_subscribe();
   }

   const fsuipc_update_observer_type& fsuipc_observer() const
   { return _update_obs; }

   fsuipc_update_observer_type& fsuipc_observer()
   { return _update_obs; }

private:

   conf::domain_settings _settings;
   fsuipc_update_observer_type _update_obs;

   void load_exports()
   {
      try
      {
         boost::property_tree::ptree pt;
         conf::bpt_auto_config_loader config_loader;
         config_loader.load_from_file(_settings.exports_file, pt);

         for (auto& entry : pt)
         {
            auto& offset_desc = entry.second;
            auto addr = offset_desc.get<oac::fsuipc::offset_address>("address");
            auto len = static_cast<oac::fsuipc::offset_length>(
                  offset_desc.get<int>("length"));
            oac::fsuipc::offset offset(addr, len);

            observe_offset(offset);
         }
      }
      catch (const conf::config_exception& e)
      {
         OAC_THROW_EXCEPTION(
               core::invalid_domain_exports(
                     _settings.name, _settings.exports_file, e));
      }
   }

   void mqtt_subscribe()
   {
      mqtt().subscribe(
            format("%s/+", FSUIPC_OFFSETS_TOPIC_PREFIX),
            mqtt::qos_level::LEVEL_0,
            std::bind(&domain::on_message, this, std::placeholders::_1));
   }

   void observe_offset(const oac::fsuipc::offset& o)
   {
      _update_obs.start_observing(o);
   }

   void on_offset_changed(const oac::fsuipc::valued_offset& o)
   {      
      mqtt().publish_as<oac::fsuipc::offset_value>(topic_for(o), o.value);
   }

   void on_message(const mqtt::raw_message& msg)
   {
      auto value = valued_offset_for(msg);
      _update_obs.update_offset(value);
   }

   mqtt::topic topic_for(const oac::fsuipc::offset& o)
   {
      mqtt::topic tpc
      {
         format("%s/%x:%d", FSUIPC_OFFSETS_TOPIC_PREFIX, o.address, o.length)
      };
      return tpc;
   }

   oac::fsuipc::valued_offset valued_offset_for(const mqtt::raw_message& msg)
   {
      auto typed = msg.to_typed<oac::fsuipc::offset_value>();
      const auto& tpc = msg.tpc.to_string();

      auto str_offset = tpc.substr(tpc.rfind('/') + 1);
      auto str_addr = str_offset.substr(0, str_offset.find(':'));
      auto str_len = str_offset.substr(str_offset.find(':') + 1);

      auto addr = offset_address_for(str_addr);
      auto len = boost::lexical_cast<std::uint32_t>(str_len);
      return oac::fsuipc::valued_offset(
            addr, static_cast<oac::fsuipc::offset_length>(len), typed.data);
   }

   oac::fsuipc::offset_address offset_address_for(const std::string& str)
   {
      oac::fsuipc::offset_address addr;
      std::stringstream ss;
      ss << std::hex << str;
      ss >> addr;
      return addr;
   }
};

template <typename FsuipcUserAdapter>
using domain_ptr = std::shared_ptr<domain<FsuipcUserAdapter>>;

/** A convenience function to create a shared FSUIPC domain. */
template <typename FsuipcUserAdapter>
inline domain_ptr<FsuipcUserAdapter>
make_domain(
      const conf::domain_settings& settings,
      const mqtt::client_ptr& mqtt_client,
      const oac::fsuipc::fsuipc_client_ptr<FsuipcUserAdapter>& fsuipc_client)
{
   return std::make_shared<domain<FsuipcUserAdapter>>(
         settings, mqtt_client, fsuipc_client);
}

/** A convenience function to create a shared FSUIPC domain. */
template <typename FsuipcUserAdapter>
inline domain_ptr<FsuipcUserAdapter>
make_domain(
      const conf::domain_settings& settings,
      const mqtt::client_ptr& mqtt_client,
      const std::shared_ptr<FsuipcUserAdapter>& fsuipc_adapter)
{
   return std::make_shared<domain<FsuipcUserAdapter>>(
         settings, mqtt_client, oac::fsuipc::make_fsuipc_client(fsuipc_adapter));
}

}}}

#endif
