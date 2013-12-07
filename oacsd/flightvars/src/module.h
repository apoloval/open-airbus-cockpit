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

#include <liboac/filesystem.h>
#include <liboac/logging.h>

#include "conf/provider.h"
#include "conf/settings.h"
#include "mqtt/broker.h"
#include "mqtt/broker_mosquitto.h"

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
         mqtt_broker_setup();
      }
      catch (oac::exception& e)
      {
         log(
               "DLLStart",
               log_level::FATAL,
               format("Unexpected error: %s", e.report()));
      }
      catch (...)
      {
         log(
               "DLLStart",
               log_level::FATAL,
               "Unexpected error: unknown exception thrown");
      }
   }

   ~flightvars_module()
   {
      try
      {
         mqtt_broker_teardown();
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

   void read_config()
   {
      // Logging is not ready: do not log
      conf::bpt_config_provider(CONFIG_DIR).load_settings(_settings);
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

   void mqtt_broker_setup()
   {
      typedef conf::mqtt_broker_runner_id_conversions conv;
      auto runner = _settings.mqtt.broker.runner;
      log_info(
            "setting up MQTT broker runner using %s... ",
            conv::to_string(runner));
      switch (_settings.mqtt.broker.runner)
      {
         case conf::mqtt_broker_runner_id::MOSQUITTO_PROCESS:
            _mbr.reset(new mqtt::mosquitto_process_runner());
            break;
         // TODO: cover other cases
      }
      if (_mbr)
         _mbr->run_broker();
      log_info("MQTT broker runner loaded successfully");
   }

   void mqtt_broker_teardown()
   {
      log_info("Shutting down MQTT broker runner... ");
      if (_mbr)
         _mbr->shutdown_broker();
      _mbr.reset();
      log_info("MQTT broker runner shut down successfully");
   }

private:

   std::unique_ptr<mqtt::broker_runner> _mbr;
   conf::flightvars_settings _settings;
};

}} // namespace oac::fv

#endif
