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

#include <boost/format.hpp>

#include "subscription.h"

namespace oac { namespace fv {

namespace {

flight_vars::subscription_id next_id = 1;

} // anonymous namespace

flight_vars::subscription_id
make_subscription_id()
{
   return next_id++;
}

void
subscription_mapper::register_subscription(
      const variable_id& var_id,
      const flight_vars::subscription_id& subs_id)
throw (duplicated_variable_error, duplicated_subscription_error)
{
   if (_map.left.find(var_id) != _map.left.end())
      boost::throw_exception(duplicated_variable_error());
   if (_map.right.find(subs_id) != _map.right.end())
      boost::throw_exception(duplicated_subscription_error());
   _map.insert(map_type::value_type(var_id, subs_id));
}

variable_id
subscription_mapper::get_var_id(
      const flight_vars::subscription_id& subs_id)
throw (unknown_subscription_error)
{
   auto entry = _map.right.find(subs_id);
   if (entry == _map.right.end())
      boost::throw_exception(unknown_subscription_error());
   return entry->second;
}

flight_vars::subscription_id
subscription_mapper::get_subscription_id(
      const variable_id& var_id)
throw (unknown_variable_error)
{
   auto entry = _map.left.find(var_id);
   if (entry == _map.left.end())
      boost::throw_exception(unknown_variable_error());
   return entry->second;
}

void
subscription_mapper::unregister(
      const variable_id& var_id)
throw (unknown_variable_error)
{
   if (_map.left.erase(var_id) == 0)
      boost::throw_exception(unknown_variable_error());
}

void
subscription_mapper::unregister(
      const flight_vars::subscription_id& subs_id)
throw (unknown_subscription_error)
{
   if (_map.right.erase(subs_id) == 0)
      boost::throw_exception(unknown_subscription_error());
}

}} // namespace oac::fv