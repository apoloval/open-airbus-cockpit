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

#ifndef OAC_FV_API_H
#define OAC_FV_API_H

#include <liboac/exception.h>

#include <flightvars/subscription.h>
#include <flightvars/var.h>

namespace oac { namespace fv {

/**
 * Flight vars API interface.
 */
class flight_vars
{
public:

   /**
    * An illegal value was provided for a variable.
    */
   OAC_DECL_ABSTRACT_EXCEPTION(illegal_value_error);

   /**
    * An illegal type was provided for a variable.
    */
   OAC_DECL_EXCEPTION_WITH_PARAMS(
         invalid_value_type_error,
         illegal_value_error,
      (
         "invalid value type %s for subscription %d",
         var_type_to_string(var_type),
         subs_id
      ),
      (subs_id, subscription_id),
      (var_type, variable_type));

   /**
    * An operation was requested on a unknown variable.
    */
   OAC_DECL_EXCEPTION_WITH_PARAMS(no_such_variable_error, oac::exception,
      ( "no such variable with id %s", var_id.to_string()),
      (var_id, variable_id));

   /**
    * An operation was requested for an unknown subscription.
    */
   OAC_DECL_EXCEPTION_WITH_PARAMS(no_such_subscription_error, oac::exception,
      ("no such subscription with id %d", subs_id),
      (subs_id, subscription_id));


   /**
    * A callback representing a subscription to a variable.
    */
   typedef std::function<void(const variable_id& id,
                              const variable_value& value)> var_update_handler;

   virtual ~flight_vars() {}

   /**
    * Subscribe to a variable.
    *
    * @param grp the variable group
    * @param name the variable name
    * @param suhandlerbs the handler to be invoked when var changes
    * @return the subscription ID, which may be used for unsubscription
    */
   virtual subscription_id subscribe(
         const variable_id& var,
         const var_update_handler& handler)
   throw (no_such_variable_error) = 0;

   /**
    * Remove the subscription with the given ID.
    *
    * @param id   The ID of the subscription to be removed
    */
   virtual void unsubscribe(const subscription_id& id)
   throw (no_such_subscription_error) = 0;

   /**
    * Update a variable by replacing its value with the given one.
    *
    * @param subs_id the ID of the subscription to the variable (previously
    *                obtained from subscribe() function)
    * @param var_value the new value of the variable
    */
   virtual void update(
         const subscription_id& subs_id,
         const variable_value& var_value)
   throw (no_such_subscription_error, illegal_value_error) = 0;
};

}} // namespace oac::fv

#endif
