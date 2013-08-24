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

std::shared_ptr<flight_vars_core>
flight_vars_core::instance()
{
   static std::shared_ptr<flight_vars_core> core(nullptr);
   if (!core)
      core = std::shared_ptr<flight_vars_core>(new flight_vars_core());
   return core;
}

subscription_id
flight_vars_core::subscribe(
      const variable_id& var,
      const var_update_handler& handler)
throw (unknown_variable_error)
{
   auto& master = get_master_by_var_id(var);
   auto id = master->subscribe(var, handler);
   _subscriptions[id] = master;
   return id;
}

void
flight_vars_core::unsubscribe(const subscription_id& id)
{
   auto entry = _subscriptions.find(id);
   if (entry != _subscriptions.end())
   {
      entry->second->unsubscribe(id);
      _subscriptions.erase(entry);
   }
}

void
flight_vars_core::update(
      const subscription_id& subs_id,
      const variable_value& var_value)
throw (unknown_variable_error, illegal_value_error)
{
   if (auto master = get_master_by_subs_id(subs_id))
      master->update(subs_id, var_value);
}

void
flight_vars_core::register_group_master(
      const variable_group& grp,
      const std::shared_ptr<flight_vars>& master)
throw (master_already_registered)
{
   auto entry = _group_masters.find(grp);
   if (entry != _group_masters.end())
      OAC_THROW_EXCEPTION(master_already_registered()
            .with_var_group_tag(grp.get_tag()));
   _group_masters[grp] = master;
}

std::shared_ptr<flight_vars>&
flight_vars_core::get_master_by_var_id(
      const variable_id& var_id)
throw (unknown_variable_error)
{
   auto grp = get_var_group(var_id);
   auto entry = _group_masters.find(grp);
   if (entry == _group_masters.end())
      OAC_THROW_EXCEPTION(unknown_variable_error()
            .with_var_group_tag(get_var_group(var_id))
            .with_var_name_tag(get_var_name(var_id)));
   return entry->second;
}

std::shared_ptr<flight_vars>
flight_vars_core::get_master_by_subs_id(
      const subscription_id& subs_id)
{
   auto entry = _subscriptions.find(subs_id);
   return (entry != _subscriptions.end()) ? entry->second : nullptr;
}

}} // namespace oac::fv
