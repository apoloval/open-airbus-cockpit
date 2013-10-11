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

#include <list>

#include <flightvars/client.h>
#include <flightvars/subscription/mapper.h>
#include <flightvars/var.h>

namespace oac { namespace fv {

class var_observer
{
public:

   typedef std::list<variable_id> variable_id_list;

   var_observer(const variable_id_list& observation);

   var_observer(const variable_id& observation);

   void set_value(
         const variable_id& var_id,
         const variable_value& var_value);

   std::future<void> disconnection()
   { return _client.disconnection(); }

private:

   flight_vars_client _client;
   subs::subscription_mapper _mapper;

   static void print_var_value(
         const variable_id& var_id,
         const variable_value& var_value);
};

}}
