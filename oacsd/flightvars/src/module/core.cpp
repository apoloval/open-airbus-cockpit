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

#include "core.h"

namespace oac { namespace fv {

ptr<flight_vars_core>
flight_vars_core::instance()
{
   static ptr<flight_vars_core> core(nullptr);
   if (!core)
      core = new flight_vars_core();
   return core;
}

flight_vars::subscription_id
flight_vars_core::subscribe(
      const variable_id& var,
      const var_update_handler& handler)
throw (unknown_variable_error)
{
   auto grp = get_var_group(var);
   auto entry = _group_masters.find(grp);
   if (entry == _group_masters.end())
      BOOST_THROW_EXCEPTION(unknown_variable_group_error() <<
            variable_group_info(grp));
   auto id = entry->second->subscribe(var, handler);
   _subscriptions[id] = entry->second;
   return id;
}

void
flight_vars_core::unsubscribe(const flight_vars::subscription_id& id)
{
   auto entry = _subscriptions.find(id);
   if (entry != _subscriptions.end())
   {
      entry->second->unsubscribe(id);
      _subscriptions.erase(entry);
   }
}

void
flight_vars_core::register_group_master(
      const variable_group& grp,
      const ptr<flight_vars>& master)
throw (master_already_registered)
{
   auto entry = _group_masters.find(grp);
   if (entry != _group_masters.end())
      BOOST_THROW_EXCEPTION(master_already_registered() <<
                  variable_group_info(grp));
   _group_masters[grp] = master;
}

}} // namespace oac::fv
