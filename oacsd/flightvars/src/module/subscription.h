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

#ifndef OAC_FV_SUBSCRIPTION_H
#define OAC_FV_SUBSCRIPTION_H

#include <boost/bimap.hpp>

#include "api.h"

namespace oac { namespace fv {

subscription_id make_subscription_id();

class subscription_mapper
{
public:

   /**
    * An unknown variable was referenced by input.
    */
   OAC_EXCEPTION_BEGIN(no_such_variable_error, oac::exception)
      OAC_EXCEPTION_FIELD(var_group_tag, variable_group::tag_type)
      OAC_EXCEPTION_FIELD(var_name_tag, variable_name::tag_type)
      OAC_EXCEPTION_MSG(
            "no such variable %s found in subscription mapper",
            var_to_string(make_var_id(var_group_tag, var_name_tag)))
   OAC_EXCEPTION_END()

   /**
    * An unknown subscription was referenced by input.
    */
   OAC_EXCEPTION_BEGIN(no_such_subscription_error, oac::exception)
      OAC_EXCEPTION_FIELD(subs_id, subscription_id)
      OAC_EXCEPTION_MSG(
            "no such subscription ID %d found in subscription mapper",
            subs_id)
   OAC_EXCEPTION_END()

   /**
    * An already existing variable was provided as input.
    */
   OAC_EXCEPTION_BEGIN(variable_already_exists_error, oac::exception)
      OAC_EXCEPTION_FIELD(var_group_tag, variable_group::tag_type)
      OAC_EXCEPTION_FIELD(var_name_tag, variable_name::tag_type)
      OAC_EXCEPTION_MSG(
            "variable %s already exists in subscription mapper",
            var_to_string(make_var_id(var_group_tag, var_name_tag)))
   OAC_EXCEPTION_END()

   /**
    * An already existing subscription was provided as input.
    */
   OAC_EXCEPTION_BEGIN(subscription_already_exists_error, oac::exception)
      OAC_EXCEPTION_FIELD(subs_id, subscription_id)
      OAC_EXCEPTION_MSG(
            "subscription ID %d already exists in subscription mapper",
            subs_id)
   OAC_EXCEPTION_END()

   /**
    * Clear all registered subscriptions.
    */
   void clear();

   /**
    * Register a new subscription.
    */
   void register_subscription(
         const variable_id& var_id,
         const subscription_id& subs_id)
   throw (variable_already_exists_error, subscription_already_exists_error);

   /**
    * Execute the given action for each mapped subscription.
    */
   void for_each_subscription(
         const std::function<void(const subscription_id&)>& action);

   /**
    * Obtain the variable ID for given subscription ID.
    * @throw no_such_subscription_error when given subscription ID is unknown
    */
   variable_id get_var_id(
         const subscription_id& subs_id)
   throw (no_such_subscription_error);

   /**
    * Obtain the subscription ID for given variable ID.
    * @throw no_such_variable_error when given variable ID is unknown
    */
   subscription_id get_subscription_id(
         const variable_id& var_id)
   throw (no_such_variable_error);

   /**
    * Unregister a mapping from its variable ID.
    * @throw no_such_variable_error when given variable ID is unknown
    */
   void unregister(
         const variable_id& var_id)
   throw (no_such_variable_error);

   /**
    * Unregister a mapping from its subscription ID.
    * @throw no_such_subscription_error when given subscription ID is unknown
    */
   void unregister(
         const subscription_id& subs_id)
   throw (no_such_subscription_error);

private:

   typedef boost::bimap<variable_id, subscription_id> map_type;

   map_type _map;
};

}} // namespace oac::fv

#endif
