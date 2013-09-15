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

#include <boost/thread/future.hpp>

#include "client/subscription_db.h"

namespace oac { namespace fv { namespace client {

subscription_id
subscription_db::create_entry(
      const variable_id& var_id,
      subscription_id master_subs_id,
      const flight_vars::var_update_handler& handler)
throw (already_exists_exception)
{
   if (variable_defined(var_id))
      OAC_THROW_EXCEPTION(variable_already_exists_error(var_id));
   if (master_subscription_defined(master_subs_id))
      OAC_THROW_EXCEPTION(
            master_subscription_already_exists_error(master_subs_id));

   init_entry(var_id, master_subs_id);

   return add_virtual_subscription(var_id, handler);
}

void
subscription_db::remove_entry(
      const variable_id& var_id)
throw (no_such_element_exception)
{
   auto e = get_entry_by_var(var_id);
   auto virtuals = get_virtual_subscriptions(var_id);
   for (auto& subs : virtuals)
      remove_virtual(subs.id);
   _master_subs_id_map.erase(e->master_subs_id);
   _var_id_map.erase(var_id);
}

bool
subscription_db::entry_defined(
      const variable_id& var_id) const
{
   return variable_defined(var_id);
}

subscription_id
subscription_db::add_virtual_subscription(
      const variable_id& var_id,
      const flight_vars::var_update_handler& handler)
throw (no_such_element_exception)
{
   if (!variable_defined(var_id))
      OAC_THROW_EXCEPTION(no_such_variable_error(var_id));
   auto virtual_subs = subscription(
         make_subscription_id(),
         handler);
   auto e = _var_id_map[var_id];
   e->virtual_subs.push_back(virtual_subs);
   _virtual_subs_id_map[virtual_subs.id] = e;
   return virtual_subs.id;
}

bool
subscription_db::remove_virtual_subscription(
      const subscription_id& virtual_subs_id)
throw (no_such_element_exception)
{
   auto e = get_entry_by_virtual(virtual_subs_id);
   if (remove_virtual(virtual_subs_id) == 0)
   {
      remove_entry(e->var_id);
      return true;
   }
   return false;
}

subscription_id
subscription_db::get_master_subscription_id(
      const variable_id& var_id)
throw (no_such_element_exception)
{
   return get_entry_by_var(var_id)->master_subs_id;
}

subscription_id
subscription_db::get_master_subscription_id(
      subscription_id virtual_subs_id)
throw (no_such_element_exception)
{
   return get_entry_by_virtual(virtual_subs_id)->master_subs_id;
}

void
subscription_db::invoke_handlers(
      const subscription_id& master_subs_id,
      const variable_value& var_value)
throw (no_such_element_exception)
{
   auto e = get_entry_by_master(master_subs_id);
   auto virtuals = get_virtual_subscriptions(e->var_id);
   for (auto& s : virtuals)
   {
      s.handler(e->var_id, var_value);
   }
}

bool
subscription_db::variable_defined(
      const variable_id& var_id) const
{
   return _var_id_map.find(var_id) != _var_id_map.end();
}

bool
subscription_db::master_subscription_defined(
      subscription_id subs_id) const
{
   return _master_subs_id_map.find(subs_id) != _master_subs_id_map.end();
}

bool
subscription_db::virtual_subscription_defined(
      subscription_id subs_id) const
{
   return _virtual_subs_id_map.find(subs_id) != _virtual_subs_id_map.end();
}

subscription_db::entry_ptr
subscription_db::init_entry(
      const variable_id& var_id,
      subscription_id master_subs_id)
{
   auto e = std::make_shared<entry>(var_id, master_subs_id);
   _var_id_map[var_id] = e;
   _master_subs_id_map[master_subs_id] = e;
   return e;
}

subscription_db::entry_ptr
subscription_db::get_entry_by_var(
      const variable_id& var_id)
throw (no_such_element_exception)
{
   if (!variable_defined(var_id))
      OAC_THROW_EXCEPTION(no_such_variable_error(var_id));
   return _var_id_map[var_id];
}

subscription_db::entry_ptr
subscription_db::get_entry_by_master(
      const subscription_id& master)
throw (no_such_element_exception)
{
   if (!master_subscription_defined(master))
      OAC_THROW_EXCEPTION(no_such_master_subscription_error(master));
   return _master_subs_id_map[master];
}

subscription_db::entry_ptr
subscription_db::get_entry_by_virtual(
      const subscription_id& virt_subs_id)
throw (no_such_element_exception)
{
   if (!virtual_subscription_defined(virt_subs_id))
      OAC_THROW_EXCEPTION(no_such_virtual_subscription_error(virt_subs_id));
   return _virtual_subs_id_map[virt_subs_id];
}

std::list<subscription_db::subscription>
subscription_db::get_virtual_subscriptions(
         const variable_id& var_id)
   throw (no_such_element_exception)
{
   return get_entry_by_var(var_id)->virtual_subs;
}

unsigned int
subscription_db::remove_virtual(
      const subscription_id& virtual_subs_id)
throw (no_such_element_exception)
{
   auto e = get_entry_by_virtual(virtual_subs_id);
   e->virtual_subs.remove_if([virtual_subs_id](const subscription& s)
   {
      return s.id == virtual_subs_id;
   });
   _virtual_subs_id_map.erase(virtual_subs_id);
   return e->virtual_subs.size();
}

}}} // namespace oac::fv::client
