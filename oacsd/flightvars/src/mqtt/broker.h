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

#ifndef OAC_FLIGHTVARS_BROKER
#define OAC_FLIGHTVARS_BROKER

#include <cstdint>
#include <string>

#include <Windows.h>

#include <liboac/exception.h>
#include <liboac/logging.h>

namespace oac { namespace fv { namespace mqtt {

OAC_DECL_ABSTRACT_EXCEPTION(broker_exception);

OAC_DECL_EXCEPTION_WITH_PARAMS(broker_run_error, broker_exception,
   (
      "cannot create subprocess with Mosquitto broker: error code %d",
      error_code
   ),
   (error_code, std::uint32_t)
);

OAC_DECL_EXCEPTION_WITH_PARAMS(broker_shutdown_error, broker_exception,
   (
      "cannot terminate subprocess with Mosquitto broker: error code %d",
      error_code
   ),
   (error_code, std::uint32_t)
);

OAC_DECL_EXCEPTION(
      no_such_broker_error,
      broker_exception,
      "cannot terminate subprocess with Mosquitto broker: no such subprocess"
);

/**
 * An object able to run a MQTT broker.
 */
class broker_runner
{
public:

   virtual void run_broker() throw (broker_exception) = 0;

   virtual void shutdown_broker() throw (broker_exception) = 0;
};

class mosquitto_process_runner :
      public oac::logger_component,
      public broker_runner
{
public:

   mosquitto_process_runner();

   virtual void run_broker() throw (broker_exception);

   virtual void shutdown_broker() throw (broker_exception);

private:

   std::wstring _command_line;
   PROCESS_INFORMATION _proc_info;
};

}}} // namespace oac::fv::mqtt

#endif
