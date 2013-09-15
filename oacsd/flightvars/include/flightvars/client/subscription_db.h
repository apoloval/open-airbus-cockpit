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
 *   virtual subscription ID). As result of entry creation, the virtual
 *   subscription ID is returned. That's the ID returned as result
 *   of the subscription() function.
 *
 * - When subscription() is called for an already subscribed variable, the
 *   client cheks that querying this DB. It creates a new virtual subscription ID
 *   for that variable and returns it. No interaction is made with the server.
 *
 * - When unsubscription() is called, this DB is requested to remove the
 *   given virtual subscription ID. If (and only if) it's the last virtual
 *   subscription, a unsubscription request is sent to the server.
 *
 * - When update() is called, this DB is queried to obtain the master
 *   subscription ID for the given virtual ID. Then it is used to send a
 *   variable update message to the server.
 *
 * - When a variable update message is received from the server, this DB
 *   is queried by variable ID to obtain the list of handlers of its virtual
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
   OAC_DECL_ABSTRACT_EXCEPTION(already_exists_exception);

   /**
    * An exception indicating an already existing variable in the DB.
    */
   OAC_DECL_EXCEPTION_WITH_PARAMS(
         variable_already_exists_error,
         already_exists_exception,
      ("Variable %s already exists in FV client DB", var_to_string(var_id)),
      (var_id, variable_id));

   /**
    * An exception indicating an already existing master subscription in the DB.
    */
   OAC_DECL_EXCEPTION_WITH_PARAMS(
         master_subscription_already_exists_error,
         already_exists_exception,
      ("Master subscription ID %d already exists in FV client DB", subs_id),
      (subs_id, subscription_id));

   /**
    * An exception indicating an element that was not found.
    */
   OAC_DECL_ABSTRACT_EXCEPTION(no_such_element_exception);

   OAC_DECL_EXCEPTION_WITH_PARAMS(
         no_such_variable_error,
         no_such_element_exception,
      ("Variable %s was not found in FV client DB", var_to_string(var_id)),
      (var_id, variable_id));

   OAC_DECL_EXCEPTION_WITH_PARAMS(
         no_such_master_subscription_error,
         no_such_element_exception,
      ("Master subscription ID %d was not found in FV client DB", subs_id),
      (subs_id, subscription_id));

   OAC_DECL_EXCEPTION_WITH_PARAMS(
         no_such_virtual_subscription_error,
         no_such_element_exception,
      ("Virtual subscription ID %d was not found in FV client DB", subs_id),
      (subs_id, subscription_id));

   /**
    * Create a new entry in the DB.
    *
    * @param var_id           The variable ID
    * @param master_subs_id   The master subscription ID
    * @param handler          The handler of the virtual subscription
    * @return                 The virtual subscription ID
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
    * Add a new virtual subscription for the entry identified by given variable.
    *
    * @param var_id  The variable that identifies the entry
    * @param handler The handler of the newly created virtual subscription
    * @return        The ID of the newly created virtual subscription
    */
   subscription_id add_virtual_subscription(
         const variable_id& var_id,
         const flight_vars::var_update_handler& handler)
   throw (no_such_element_exception);

   /**
    * Remove the virtual subscription identified by given ID and indicate whether
    * the entry was removed due to it had no more virtual subscriptions.
    *
    * @param virtual_subs_id    The ID of the virtual subscription to be removed
    * @return                 True if the entry was removed
    */
   bool remove_virtual_subscription(
         const subscription_id& virtual_subs_id)
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
    * Obtain the master subscription ID for given virtual subscription.
    *
    * @param virtual_subs_id The ID of the virtual subscription whose master
    *                      subscription is seek
    * @return              The ID of the master subscription
    */
   subscription_id get_master_subscription_id(
         subscription_id virtual_subs_id)
   throw (no_such_element_exception);

   /**
    * Invoke the handlers of the virtual subscriptions for given master
    * subscription.
    *
    * @param master_subs_id   The ID of the master subscription whose virtuals
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
      std::list<subscription> virtual_subs;

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
   std::unordered_map<subscription_id, entry_ptr> _virtual_subs_id_map;

   bool variable_defined(
         const variable_id& var_id) const;

   bool master_subscription_defined(
         subscription_id subs_id) const;

   bool virtual_subscription_defined(
         subscription_id subs_id) const;

   entry_ptr init_entry(
         const variable_id& var_id,
         subscription_id master_subs_id);

   entry_ptr get_entry_by_var(
         const variable_id& var_id)
   throw (no_such_element_exception);

   entry_ptr get_entry_by_master(
         const subscription_id& virt_subs_id)
   throw (no_such_element_exception);

   entry_ptr get_entry_by_virtual(
         const subscription_id& virt_subs_id)
   throw (no_such_element_exception);

   std::list<subscription> get_virtual_subscriptions(
         const variable_id& var_id)
   throw (no_such_element_exception);

   unsigned int remove_virtual(
         const subscription_id& virtual_subs_id)
   throw (no_such_element_exception);
};

}}} // namespace oac::fv::client

#endif
