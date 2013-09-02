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

#ifndef OAC_FV_CLIENT_SUBSCRIPTION_DB_H
#define OAC_FV_CLIENT_SUBSCRIPTION_DB_H

#include <list>
#include <unordered_map>

#include <liboac/logging.h>
#include <liboac/network.h>

#include "api.h"
#include "protocol.h"
#include "subscription.h"

namespace oac { namespace fv { namespace client {

/**
 * An in-memory tiny database for storing FlightVars client data.
 *
 * This DB stores the relationship between the different entities required by
 * FV client to work. The client implements the flight_vars API, and therefore
 * it must allow different subscriptions for the same variable. The FV
 * server architecture establishes the contrary: only one subscription is
 * allowed per variable. In order to tie thethings, the client proceeds as
 * follows.
 *
 * - When subscription() is called for a new variable, the client sends the
 *   request to the server. After returning the control, it creates a entry
 *   in this DB with the var ID, the subscription ID assigned by the server
 *   (known as master subscription ID) and a newly generated subscription ID
 *   used to identify the subscription from the client perspective (known as
 *   slave subscription ID). As result of entry creation, the slave
 *   subscription ID is returned. That's the ID returned as result
 *   of the subscription() function.
 *
 * - When subscription() is called for an already subscribed variable, the
 *   client cheks that querying this DB. It creates a new slave subscription ID
 *   for that variable and returns it. No interaction is made with the server.
 *
 * - When unsubscription() is called, this DB is requested to remove the
 *   given slave subscription ID. If (and only if) it's the last slave
 *   subscription, a unsubscription request is sent to the server.
 *
 * - When update() is called, this DB is queried to obtain the master
 *   subscription ID for the given slave ID. Then it is used to send a
 *   variable update message to the server.
 *
 * - When a variable update message is received from the server, this DB
 *   is queried by variable ID to obtain the list of handlers of its slave
 *   subscriptions. Then the handlers are invoked.
 *
 * Once this procedure is understood, it's easy to understand the purpose of
 * the model and the primities this DB offers.
 */
class subscription_db
{
public:

   /**
    * An exception indicating an already existing element in the DB.
    */
   OAC_ABSTRACT_EXCEPTION(already_exists_exception);

   /**
    * An exception indicating an already existing variable in the DB.
    */
   OAC_EXCEPTION_BEGIN(variable_already_exists_error, already_exists_exception)
      OAC_EXCEPTION_FIELD(var_group_tag, variable_group::tag_type)
      OAC_EXCEPTION_FIELD(var_name_tag, variable_name::tag_type)
      OAC_EXCEPTION_MSG(
            "Variable %s already exists in FV client DB",
            var_to_string(make_var_id(var_group_tag, var_name_tag)))
   OAC_EXCEPTION_END()

   /**
    * An exception indicating an already existing master subscription in the DB.
    */
   OAC_EXCEPTION_BEGIN(
         master_subscription_already_exists_error,
         already_exists_exception)
      OAC_EXCEPTION_FIELD(subs_id, subscription_id)
      OAC_EXCEPTION_MSG(
            "Master subscription ID %d already exists in FV client DB",
            subs_id)
   OAC_EXCEPTION_END()

   /**
    * An exception indicating an element that was not found.
    */
   OAC_ABSTRACT_EXCEPTION(no_such_element_exception);

   OAC_EXCEPTION_BEGIN(no_such_variable_error, no_such_element_exception)
      OAC_EXCEPTION_FIELD(var_group_tag, variable_group::tag_type)
      OAC_EXCEPTION_FIELD(var_name_tag, variable_name::tag_type)
      OAC_EXCEPTION_MSG(
            "Variable %s was not found in FV client DB",
            var_to_string(make_var_id(var_group_tag, var_name_tag)))
   OAC_EXCEPTION_END()

   OAC_EXCEPTION_BEGIN(
         no_such_master_subscription_error,
         no_such_element_exception)
      OAC_EXCEPTION_FIELD(subs_id, subscription_id)
      OAC_EXCEPTION_MSG(
            "Master subscription ID %d was not found in FV client DB",
            subs_id)
   OAC_EXCEPTION_END()

   OAC_EXCEPTION_BEGIN(
         no_such_slave_subscription_error,
         no_such_element_exception)
      OAC_EXCEPTION_FIELD(subs_id, subscription_id)
      OAC_EXCEPTION_MSG(
            "Slave subscription ID %d was not found in FV client DB",
            subs_id)
   OAC_EXCEPTION_END()

   /**
    * Create a new entry in the DB.
    *
    * @param var_id           The variable ID
    * @param master_subs_id   The master subscription ID
    * @param handler          The handler of the slave subscription
    * @return                 The slave subscription ID
    */
   subscription_id create_entry(
         const variable_id& var_id,
         subscription_id master_subs_id,
         const flight_vars::var_update_handler& handler)
   throw (already_exists_exception);

   /**
    * Remove an entry from this DB.
    *
    * @param var_id  The variable that identifies the entry.
    */
   void remove_entry(
         const variable_id& var_id)
   throw (no_such_element_exception);

   /**
    * Check whether entry is defined.
    *
    * @param var_id  The variable ID for the entry to be checked
    * @return        True if entry is defined, false othewise
    */
   bool entry_defined(const variable_id& var_id) const;

   /**
    * Add a new slave subscription for the entry identified by given variable.
    *
    * @param var_id  The variable that identifies the entry
    * @param handler The handler of the newly created slave subscription
    * @return        The ID of the newly created slave subscription
    */
   subscription_id add_slave_subscription(
         const variable_id& var_id,
         const flight_vars::var_update_handler& handler)
   throw (no_such_element_exception);

   /**
    * Remove the slave subscription identified by given ID and indicate whether
    * the entry was removed due to it had no more slave subscriptions.
    *
    * @param slave_subs_id    The ID of the slave subscription to be removed
    * @return                 True if the entry was removed
    */
   bool remove_slave_subscription(
         const subscription_id& slave_subs_id)
   throw (no_such_element_exception);

   /**
    * Obtain the master subscription ID for given variable.
    *
    * @param var_id  The ID of the variable whose master subscription is seek
    * @return        The ID of the master subscription
    */
   subscription_id get_master_subscription_id(
         const variable_id& var_id)
   throw (no_such_element_exception);

   /**
    * Obtain the master subscription ID for given slave subscription.
    *
    * @param slave_subs_id The ID of the slave subscription whose master
    *                      subscription is seek
    * @return              The ID of the master subscription
    */
   subscription_id get_master_subscription_id(
         subscription_id slave_subs_id)
   throw (no_such_element_exception);

   /**
    * Invoke the handlers of the slave subscriptions for given master
    * subscription.
    *
    * @param master_subs_id   The ID of the master subscription whose slaves
    *                         handlers are to be invoked
    * @param var_value        The value passed to the handlers
    */
   void invoke_handlers(
         const subscription_id& master_subs_id,
         const variable_value& var_value)
   throw (no_such_element_exception);

private:

   struct subscription
   {
      subscription_id id;
      flight_vars::var_update_handler handler;

      subscription(
            subscription_id id,
            const flight_vars::var_update_handler& handler)
         : id(id),
           handler(handler)
      {}
   };

   struct entry
   {
      variable_id var_id;
      subscription_id master_subs_id;
      std::list<subscription> slave_subs;

      entry(
            const variable_id& var,
            subscription_id master_subs)
         : var_id(var),
           master_subs_id(master_subs)
      {}
   };

   typedef std::shared_ptr<entry> entry_ptr;

   std::unordered_map<variable_id, entry_ptr, variable_id_hash> _var_id_map;
   std::unordered_map<subscription_id, entry_ptr> _master_subs_id_map;
   std::unordered_map<subscription_id, entry_ptr> _slave_subs_id_map;

   bool variable_defined(
         const variable_id& var_id) const;

   bool master_subscription_defined(
         subscription_id subs_id) const;

   bool slave_subscription_defined(
         subscription_id subs_id) const;

   entry_ptr init_entry(
         const variable_id& var_id,
         subscription_id master_subs_id);

   entry_ptr get_entry_by_var(
         const variable_id& var_id)
   throw (no_such_element_exception);

   entry_ptr get_entry_by_master(
         const subscription_id& slave)
   throw (no_such_element_exception);

   entry_ptr get_entry_by_slave(
         const subscription_id& slave)
   throw (no_such_element_exception);

   std::list<subscription> get_slave_subscriptions(
         const variable_id& var_id)
   throw (no_such_element_exception);

   unsigned int remove_slave(
         const subscription_id& slave_subs_id)
   throw (no_such_element_exception);
};

}}} // namespace oac::fv::client

#endif
