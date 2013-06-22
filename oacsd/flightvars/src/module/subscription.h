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

flight_vars::subscription_id make_subscription_id();

class subscription_mapper
{
public:

   /**
    * An unknown variable was referenced by input.
    */
   OAC_DECL_ERROR(unknown_variable_error, invalid_input_error);

   /**
    * An unknown subscription was referenced by input.
    */
   OAC_DECL_ERROR(unknown_subscription_error, invalid_input_error);

   /**
    * An already existing variable was provided as input.
    */
   OAC_DECL_ERROR(duplicated_variable_error, invalid_input_error);

   /**
    * An already existing subscription was provided as input.
    */
   OAC_DECL_ERROR(duplicated_subscription_error, invalid_input_error);

   void register_subscription(
         const variable_id& var_id,
         const flight_vars::subscription_id& subs_id)
   throw (duplicated_variable_error, duplicated_subscription_error);

   /**
    * Obtain the variable ID for given subscription ID.
    * @throw unknown_subscription_error when given subscription ID is unknown
    */
   variable_id get_var_id(
         const flight_vars::subscription_id& subs_id)
   throw (unknown_subscription_error);

   /**
    * Obtain the subscription ID for given variable ID.
    * @throw unknown_variable_error when given variable ID is unknown
    */
   flight_vars::subscription_id get_subscription_id(
         const variable_id& var_id)
   throw (unknown_variable_error);

   /**
    * Unregister a mapping from its variable ID.
    * @throw unknown_variable_error when given variable ID is unknown
    */
   void unregister(
         const variable_id& var_id)
   throw (unknown_variable_error);

   /**
    * Unregister a mapping from its subscription ID.
    * @throw unknown_subscription_error when given subscription ID is unknown
    */
   void unregister(
         const flight_vars::subscription_id& subs_id)
   throw (unknown_subscription_error);

private:

   typedef boost::bimap<variable_id, flight_vars::subscription_id> map_type;

   map_type _map;
};

}} // namespace oac::fv

#endif
