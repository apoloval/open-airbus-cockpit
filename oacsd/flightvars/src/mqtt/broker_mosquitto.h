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

#ifndef OAC_FV_BROKER_MOSQUITTO
#define OAC_FV_BROKER_MOSQUITTO

#include <cstdint>
#include <string>

#include <Windows.h>

#include <liboac/logging.h>

#include "mqtt/broker.h"

namespace oac { namespace fv { namespace mqtt {

OAC_DECL_EXCEPTION_WITH_PARAMS(broker_run_error, broker_exception,
   (
      "cannot create subprocess with Mosquitto broker: error code %d",
      error_code
   ),
   (error_code, int)
);

OAC_DECL_EXCEPTION_WITH_PARAMS(broker_shutdown_error, broker_exception,
   (
      "cannot terminate subprocess with Mosquitto broker: error code %d",
      error_code
   ),
   (error_code, int)
);

OAC_DECL_EXCEPTION(
      no_such_broker_error,
      broker_exception,
      "cannot terminate subprocess with Mosquitto broker: no such subprocess"
);

class mosquitto_process_runner final :
      public logger_component,
      public broker_runner
{
public:

   mosquitto_process_runner();

   void run_broker() throw (broker_exception) override;

   void shutdown_broker() throw (broker_exception) override;

private:

   std::wstring _command_line;
   PROCESS_INFORMATION _proc_info;

   void test_shutdown_action(
         const std::string& action_des,
         std::function<bool(void)> action);
};

}}} // namespace oac::fv::mqtt

#endif
