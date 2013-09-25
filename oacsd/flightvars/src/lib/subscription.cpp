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
 * along with Open Airbus Cockpit. If not, see <http://www.gnu.org/licenses/>.
 */

#include <flightvars/subscription.h>

namespace oac { namespace fv {

namespace {

subscription_id next_id = 1;

} // anonymous namespace

subscription_id
make_subscription_id()
{
   return next_id++;
}

void
subscription_mapper::clear()
{
   _map.clear();
}

bool
subscription_mapper::subscription_exists(
      const variable_id& var_id) const
{
   return (_map.left.find(var_id) != _map.left.end());
}

bool
subscription_mapper::subscription_exists(
      const subscription_id& subs_id) const
{
   return (_map.right.find(subs_id) != _map.right.end());
}

void
subscription_mapper::register_subscription(
      const variable_id& var_id,
      const subscription_id& subs_id)
throw (variable_already_exists_error, subscription_already_exists_error)
{
   if (subscription_exists(var_id))
      OAC_THROW_EXCEPTION(variable_already_exists_error(var_id));
   if (subscription_exists(subs_id))
      OAC_THROW_EXCEPTION(subscription_already_exists_error(subs_id));
   _map.insert(map_type::value_type(var_id, subs_id));
}

void
subscription_mapper::for_each_subscription(
      const std::function<void(const subscription_id&)>& action)
{
   for (auto& entry : _map.right)
   {
      action(entry.first);
   }
}

variable_id
subscription_mapper::get_var_id(
      const subscription_id& subs_id)
throw (no_such_subscription_error)
{
   auto entry = _map.right.find(subs_id);
   if (entry == _map.right.end())
      OAC_THROW_EXCEPTION(no_such_subscription_error(subs_id));
   return entry->second;
}

subscription_id
subscription_mapper::get_subscription_id(
      const variable_id& var_id)
throw (no_such_variable_error)
{
   auto entry = _map.left.find(var_id);
   if (entry == _map.left.end())
      OAC_THROW_EXCEPTION(no_such_variable_error(var_id));
   return entry->second;
}

void
subscription_mapper::unregister(
      const variable_id& var_id)
throw (no_such_variable_error)
{
   if (_map.left.erase(var_id) == 0)
      OAC_THROW_EXCEPTION(no_such_variable_error(var_id));
}

void
subscription_mapper::unregister(
      const subscription_id& subs_id)
throw (no_such_subscription_error)
{
   if (_map.right.erase(subs_id) == 0)
      OAC_THROW_EXCEPTION(no_such_subscription_error(subs_id));
}

}} // namespace oac::fv
