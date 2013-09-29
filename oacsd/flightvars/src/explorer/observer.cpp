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

#include "observer.h"

namespace oac { namespace fv {

var_observer::var_observer(
      const variable_id_list& observation)
   : _client("FlightVars Explorer", "localhost", 8642)
{
   for (auto& var : observation)
   {
      std::cerr << "Subscribing to variable " << var.to_string() << std::endl;
      auto subs_id =_client.subscribe(
            var,
            std::bind(
                  &var_observer::print_var_value,
                  std::placeholders::_1,
                  std::placeholders::_2));
      _mapper.register_subscription(var, subs_id);
      std::cerr <<
            format("Subscribed to %s successfully", var.to_string()) <<
            std::endl;
   }
}

var_observer::var_observer(
      const variable_id& observation)
   : var_observer(variable_id_list(1, observation))
{
}

void
var_observer::set_value(
      const variable_id& var_id,
      const variable_value& var_value)
{
   auto subs_id = _mapper.get_subscription_id(var_id);
   _client.update(subs_id, var_value);
}

void
var_observer::print_var_value(
      const variable_id& var_id,
      const variable_value& var_value)
{
   std::cout <<
         format(
               "New value for variable %s: %s",
               var_id.to_string(),
               var_value.to_string()) <<
         std::endl;
}

}}
