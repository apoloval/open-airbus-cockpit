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
#include <liboac/thread/task.h>

#include "conf/exports_fsuipc.h"
#include "conf/settings.h"
#include "core/domain.h"
#include "mqtt/message.h"

namespace oac { namespace fv { namespace fsuipc {

extern const std::string FSUIPC_OFFSETS_TOPIC_PREFIX;

template <typename T>
struct message
{
   std::uint8_t sender;
   std::uint8_t reserved;
   T value;
};

using byte_message = message<std::uint8_t>;
using word_message = message<std::uint16_t>;
using dword_message = message<std::uint32_t>;

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
      },
      _executor { std::make_shared<thread::task_executor>() },
      _polling_task
      {
         _executor,
         std::bind(
               &fsuipc_update_observer_type::check_for_updates,
               &_update_obs),
         std::chrono::milliseconds(250)
      }
   {
      load_exports();
      mqtt_subscribe();
   }

   ~domain() { stop(); }

   bool is_running() const
   {
      return _executor_thread.get_id() != std::thread::id() &&
            _polling_task.is_running();
   }

   void start()
   {
      if (!is_running())
      {
         log_info("starting FSUIPC Offsets domain");
         _executor_thread = std::thread
         { std::bind(&thread::task_executor::loop, _executor) };
         _polling_task.start();
         log_info("FSUIPC Offsets domain started successfully");
      }
   }

   void stop()
   {
      if (is_running())
      {
         log_info("stopping domain FSUIPC Offsets");
         _polling_task.stop();
         _executor->stop_loop();
         _executor_thread.join();
         _executor_thread = std::thread();
         log_info("FSUIPC Offsets domain stopped successfully");
      }
   }

private:

   using opt_valued_offset = boost::optional<oac::fsuipc::valued_offset>;
   using opt_offset_address = boost::optional<oac::fsuipc::offset_address>;
   using opt_offset_length = boost::optional<oac::fsuipc::offset_length>;

   conf::domain_settings _settings;
   fsuipc_update_observer_type _update_obs;
   thread::task_executor_ptr _executor;
   std::thread _executor_thread;
   thread::recurrent_task<void> _polling_task;

   void load_exports()
   {
      std::list<oac::fsuipc::offset> offsets;
      conf::load_exports(_settings, offsets);

      for (auto& offset : offsets)
      {
         log_info("loading export for offset 0x%x:%d",
               offset.address, offset.length);
         observe_offset(offset);
      }
   }

   void mqtt_subscribe()
   {
      // Please note that callback provided to MQTT for incoming messages
      // is not directly invoking the on_message() function. Instead, it
      // submits such invocation as a task to execute, which will invoke
      // it from the very thread that is polling FSUIPC. That eliminates
      // race conditions in the update observer object.
      mqtt().subscribe(
            format("%s/+", FSUIPC_OFFSETS_TOPIC_PREFIX),
            mqtt::qos_level::LEVEL_0,
            [this](const mqtt::raw_message& msg)
            {
               _executor->submit_task<void>(
                     std::bind(&domain::on_message, this, msg));
            });
   }

   void observe_offset(const oac::fsuipc::offset& o)
   {
      _update_obs.start_observing(o);
   }

   void on_offset_changed(const oac::fsuipc::valued_offset& o)
   {
      log_trace("change detected in offset 0x%x:%d", o.address, o.length);
      switch (o.length)
      {
         case oac::fsuipc::OFFSET_LEN_BYTE:
         {
            byte_message msg { 0, 0, static_cast<std::uint8_t>(o.value) };
            mqtt().publish_as<byte_message>(topic_for(o), msg);
            break;
         }
         case oac::fsuipc::OFFSET_LEN_WORD:
         {
            word_message msg { 0, 0, static_cast<std::uint16_t>(o.value) };
            mqtt().publish_as<word_message>(topic_for(o), msg);
            break;
         }
         case oac::fsuipc::OFFSET_LEN_DWORD:
         {
            dword_message msg { 0, 0, o.value };
            mqtt().publish_as<dword_message>(topic_for(o), msg);
            break;
         }
      }
   }

   void on_message(const mqtt::raw_message& msg)
   {
      auto value = valued_offset_for(msg);
      if (value)
      {
         log_trace(
               "MQTT message requesting update on offset 0x%x:%d with value %d",
               value->address, value->length, value->value);
         _update_obs.update_offset(*value);
      }
   }

   mqtt::topic topic_for(const oac::fsuipc::offset& o)
   {
      mqtt::topic tpc
      {
         format("%s/%x:%d", FSUIPC_OFFSETS_TOPIC_PREFIX, o.address, o.length)
      };
      return tpc;
   }

   opt_valued_offset valued_offset_for(const mqtt::raw_message& msg)
   {
      const auto& tpc = msg.tpc.to_string();
      try
      {
         auto str_offset = tpc.substr(tpc.rfind('/') + 1);
         auto str_addr = str_offset.substr(0, str_offset.find(':'));
         auto str_len = str_offset.substr(str_offset.find(':') + 1);

         auto addr = offset_address_for(str_addr);
         auto len = offset_length_for(str_len);
         if (addr && len)
         {
            switch (*len)
            {
               case oac::fsuipc::OFFSET_LEN_BYTE:
                  return extract_from_message<std::uint8_t>(*addr, *len, msg);
               case oac::fsuipc::OFFSET_LEN_WORD:
                  return extract_from_message<std::uint16_t>(*addr, *len, msg);
               case oac::fsuipc::OFFSET_LEN_DWORD:
                  return extract_from_message<std::uint32_t>(*addr, *len, msg);
            }
         }
      }
      catch (boost::bad_lexical_cast&)
      {
         log_warn("received message with invalid topic %s", tpc);
      }
      return opt_valued_offset();
   }

   opt_offset_address offset_address_for(const std::string& str)
   {
      std::int32_t addr;
      std::stringstream ss;
      ss << std::hex << str;
      ss >> addr;
      if (0 < addr && addr < 0xcccc)
         return addr;
      else
      {
         log_warn("receive message with invalid offset address %s", str);
         return opt_offset_address();
      }
   }

   opt_offset_length offset_length_for(const std::string& str)
   {
      auto len = boost::lexical_cast<std::uint32_t>(str);
      switch (len)
      {
         case 1: return oac::fsuipc::OFFSET_LEN_BYTE;
         case 2: return oac::fsuipc::OFFSET_LEN_WORD;
         case 4: return oac::fsuipc::OFFSET_LEN_DWORD;
         default:
            log_warn("received message with invalid offset length %s", str);
            return opt_offset_length();
      }
   }

   template <typename T>
   opt_valued_offset extract_from_message(
         const oac::fsuipc::offset_address& addr,
         const oac::fsuipc::offset_length& len,
         const mqtt::raw_message& msg)
   {
      try
      {
         auto typed = msg.to_typed<message<T>>();
         // no sender means own message; ignore it
         if (typed.data.sender)
            return oac::fsuipc::valued_offset { addr, len, typed.data.value };
      }
      catch (const mqtt::raw_message::conversion_error& e)
      {
         log_warn("received a message with invalid payload: %s", e.report());
      }
      return opt_valued_offset {};
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
