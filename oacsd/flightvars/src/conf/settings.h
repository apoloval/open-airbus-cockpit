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

#ifndef OAC_FV_CONF_SETTINGS_H
#define OAC_FV_CONF_SETTINGS_H

#include <string>

#include <boost/filesystem/path.hpp>
#include <liboac/logging.h>
#include <liboac/util/enum.h>

namespace oac { namespace fv { namespace conf {

enum class mqtt_broker_runner_id
{
   MOSQUITTO_PROCESS,
   MOSQUITTO_SERVICE,
};

OAC_DECL_ENUM_CONVERSIONS(mqtt_broker_runner_id,
   mqtt_broker_runner_id::MOSQUITTO_PROCESS, "mosquitto-process",
   mqtt_broker_runner_id::MOSQUITTO_SERVICE, "mosquitto-service"
);

struct flightvars_settings
{
   static const mqtt_broker_runner_id DEFAULT_MQTT_BROKER_RUNNER;
   static const log_level DEFAULT_LOG_LEVEL;

   static boost::filesystem::path default_log_file();

   struct {
      bool enabled;
      boost::filesystem::path file;
      log_level level;
   } logging;
   struct {
      struct
      {
         mqtt_broker_runner_id runner;
      } broker;
   } mqtt;
};

}}} // namespace oac::fv::conf

#endif
