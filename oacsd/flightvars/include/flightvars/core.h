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

#ifndef OAC_FV_CORE_H
#define OAC_FV_CORE_H

#include <map>

#include "api.h"

namespace oac { namespace fv {

/**
 * Flight vars core object. The Flight Vars core object is the central
 * component of the Flight Vars module which registers the capabilities
 * of Flight Vars. It is implemented as a singleton so any other module
 * running in the simulator is able to interact with the core object in
 * order to register a new variable group master.
 */
class flight_vars_core : public flight_vars
{
public:

   /**
    * Thrown in a group master registering attempt when there is already
    * a master registered for that group.
    */
   OAC_EXCEPTION_BEGIN(master_already_registered, oac::exception)
      OAC_EXCEPTION_FIELD(var_group_tag, variable_group::tag_type)
      OAC_EXCEPTION_MSG(
            "FlightVars Master for group %s already registered",
            var_group_tag)
   OAC_EXCEPTION_END()

   /**
    * Obtain the singleton instance of Flight Vars core object.
    */
   static std::shared_ptr<flight_vars_core> instance();

   virtual subscription_id subscribe(
         const variable_id& var,
         const var_update_handler& handler)
   throw (no_such_variable_error);

   virtual void unsubscribe(
         const subscription_id& id)
   throw (no_such_subscription_error);

   virtual void update(
         const subscription_id& subs_id,
         const variable_value& var_value)
   throw (no_such_subscription_error, illegal_value_error);

   /**
    * Register a master for given variable group. If there is already a
    * master for given group, a master_already_registered is thrown.
    */
   void register_group_master(
         const variable_group& grp,
         const std::shared_ptr<flight_vars>& master)
   throw (master_already_registered);

private:

   typedef std::map<
         variable_group::tag_type,
         std::shared_ptr<flight_vars>> group_master_dict;
   typedef std::map<
         subscription_id,
         std::shared_ptr<flight_vars>> subscription_master_dict;

   group_master_dict _group_masters;
   subscription_master_dict _subscriptions;

   flight_vars_core() {}

   std::shared_ptr<flight_vars>& get_master_by_var_id(
         const variable_id& var_id)
   throw (no_such_variable_error);

   std::shared_ptr<flight_vars> get_master_by_subs_id(
         const subscription_id& subs_id);
};

}} // namespace oac::fv

#endif
