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

#ifndef OAC_FV_FLIGHTVARS_MODULE_H
#define OAC_FV_FLIGHTVARS_MODULE_H

#include <codecvt>

#include <liboac/filesystem.h>
#include <liboac/logging.h>
#include <liboac/thread/task.h>

#include "conf/provider.h"
#include "conf/settings.h"
#include "core/domain.h"
#include "fsuipc/domain.h"
#include "mqtt/broker.h"
#include "mqtt/broker_mosquitto.h"
#include "mqtt/client.h"
#include "mqtt/client_mosquitto.h"

#define LOG_FILE "C:\\Windows\\Temp\\FlightVars.log"
#define CONFIG_DIR "C:\\ProgramData\\OACSD"

namespace oac { namespace fv {

class flightvars_module : public logger_component
{
public:

   flightvars_module() : logger_component("flightvars_module")
   {
      try
      {
         read_config();
         logging_setup();
         log_info("initializing FlightVars module... ");
         task_executor_setup();
         mqtt_broker_setup();
         mqtt_client_setup();
         domain_setup();
         log_info("FlightVars module successfully initialized");
      }
      catch (oac::exception& e)
      {         
         auto msg = format("Unexpected error: %s", e.report());
         popup_message(msg);
         log("DLLStart", log_level::FATAL, msg);
      }
      catch (...)
      {
         auto msg = "Unexpected error: unknown exception thrown";
         popup_message(msg);
         log("DLLStart", log_level::FATAL, msg);
      }
   }

   ~flightvars_module()
   {
      try
      {
         log_info("terminating FlightVars module... ");
         domain_teardown();
         mqtt_client_teardown();
         mqtt_broker_teardown();
         task_executor_teardown();
         log_info("FlightVars module successfully terminated");
      }
      catch (oac::exception& e)
      {
         log(
               "DLLStop",
               log_level::FATAL,
               format("Unexpected error: %s", e.report()));
      }
      catch (...)
      {
         log(
               "DLLStop",
               log_level::FATAL,
               "Unexpected error: unknown exception thrown");
      }
   }

private:

   void popup_message(const std::string& msg)
   {
      std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
      MessageBox(
            NULL,
            converter.from_bytes(msg).c_str(),
            L"FlightVars warning",
            MB_OK | MB_ICONEXCLAMATION);
   }

   void read_config()
   {
      // Logging is not ready: do not log
      try
      {
         conf::bpt_config_provider(CONFIG_DIR).load_settings(_settings);
      }
      catch (const conf::config_exception& e)
      {
         auto msg = oac::format(
               "Unexpected error occured while reading FlightVars config "
               "from %s: %s! Please correct the config file and run again.",
               CONFIG_DIR,
               e.message());
         popup_message(msg);
         // Rethrow the exception and leave the plugin uninitialized
         throw;
      }
   }

   void logging_setup()
   {
      // Logging is not ready: do not log
      if (_settings.logging.enabled)
      {
         file log_file(_settings.logging.file);
         auto logger = make_logger(_settings.logging.level, log_file.append());
         set_main_logger(logger);
      }
   }

   void task_executor_setup()
   {
      log_info("initializing task executor...");
      _executor = std::make_shared<thread::task_executor>();
      _executor_thread = std::thread
      { std::bind(&thread::task_executor::loop, _executor) };
      log_info("task executor successfully initialized");
   }

   void task_executor_teardown()
   {
      log_info("shutting down task executor");
      _executor->stop_loop();
      _executor_thread.join();
      _executor.reset();
      log_info("task executor shut down successfully");
   }

   void mqtt_broker_setup()
   {
      typedef conf::mqtt_broker_runner_id_conversions conv;
      auto runner = _settings.mqtt.broker.runner;
      switch (runner)
      {
         case conf::mqtt_broker_runner_id::MOSQUITTO_PROCESS:
            _mqtt_runner.reset(new mqtt::mosquitto_process_runner());
            break;
      }
      if (_mqtt_runner)
      {
         log_info(
               "running MQTT broker using runner %s... ",
               conv::to_string(runner));
         _mqtt_runner->run_broker();
         log_info("MQTT broker run successfully");
      }
      else
         log_info("no MQTT broker runner configured: broker run omitted");
   }

   void mqtt_broker_teardown()
   {
      if (_mqtt_runner)
      {
         log_info("shutting down MQTT broker runner... ");
         _mqtt_runner->shutdown_broker();
         _mqtt_runner.reset();
         log_info("MQTT broker runner shut down successfully");
      }
      else
         log_info(
               "no MQTT broker runner was configured: broker shutdown omitted");
   }

   void mqtt_client_setup()
   {
      auto client = _settings.mqtt.client;
      log_info("initializing MQTT client using %s implementation",
            conf::mqtt_client_id_conversions::to_string(client));
      switch (client)
      {
         case conf::mqtt_client_id::DEFAULT:
         case conf::mqtt_client_id::MOSQUITTO:
            _mqtt_client.reset(new mqtt::mosquitto_client());
            break;
      }
      log_info("MQTT client initialized successfully");
   }

   void mqtt_client_teardown()
   {
      if (_mqtt_client)
      {
         log_info("shutting down MQTT client... ");
         _mqtt_client.reset();
         log_info("MQTT client shut down successfully");
      }
      else
         log_info("no MQTT client was configured: client shutdown omitted");
   }

   void domain_setup()
   {
      for (auto& dom_setts : _settings.domains)
      {
         switch (dom_setts.type)
         {
            case conf::domain_type::FSUIPC_OFFSETS:
               fsuipc_domain_setup(dom_setts);
               break;
            case conf::domain_type::CUSTOM:
               log_warn("cannot set up domain %s: %s",
                     dom_setts.name,
                     "custom domains are not supported yet");
               break;
         }
      }
   }

   void fsuipc_domain_setup(const conf::domain_settings& dom_setts)
   {
      log_info("setting up domain %s...", dom_setts.name);
      _domains.fsuipc = fsuipc::make_domain(
            dom_setts,
            _mqtt_client,
            oac::fsuipc::make_fsuipc_client(
                  std::make_shared<oac::fsuipc::local_user_adapter>()));
      _domains.fsuipc_update_task =
      {
         _executor,
         [this]()
         {
            _domains.fsuipc->fsuipc_observer().check_for_updates();
         },
         std::chrono::milliseconds(250)
      };
      _domains.fsuipc_update_task.start();
      log_info("domain %s set up as FSUIPC Offsets successfully",
            dom_setts.name);
   }

   void domain_teardown()
   {
      if (_domains.fsuipc)
      {
         log_info("shutting down FSUIPC Offsets domain...");
         _domains.fsuipc_update_task.stop();
         _domains.fsuipc.reset();         
         log_info("FSUIPC Offsets domain shut down successfully");
      }
   }

private:

   using fsuipc_domain_ptr =
         fsuipc::domain_ptr<oac::fsuipc::local_user_adapter>;

   conf::flightvars_settings _settings;
   mqtt::broker_runner_ptr _mqtt_runner;
   mqtt::client_ptr _mqtt_client;
   thread::task_executor_ptr _executor;
   std::thread _executor_thread;
   struct
   {
      fsuipc_domain_ptr fsuipc;
      thread::recurrent_task<void> fsuipc_update_task;
   } _domains;
};

}} // namespace oac::fv

#endif
