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

#ifndef OAC_FV_FSUIPC_H
#define OAC_FV_FSUIPC_H

#include <list>
#include <map>
#include <unordered_map>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <boost/bimap.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <liboac/fsuipc.h>
#include <liboac/lang-utils.h>
#include <liboac/logging.h>
#include <liboac/simconn.h>

#include "api.h"
#include "subscription.h"

namespace oac { namespace fv {

namespace fsuipc {

/**
 * An exception indicating a non-fsuipc variable ID.
 */
OAC_ABSTRACT_EXCEPTION(invalid_var_exception);

/**
 * The given var group is not FSUIPC offset.
 */
OAC_EXCEPTION_BEGIN(invalid_var_group_error, invalid_var_exception)
   OAC_EXCEPTION_FIELD(expected_var_group_tag, variable_group::tag_type)
   OAC_EXCEPTION_FIELD(actual_var_group_tag, variable_group::tag_type)
   OAC_EXCEPTION_MSG(
         "invalid variable group %s for FSUIPC Flight Vars (%s was expected)",
         actual_var_group_tag,
         expected_var_group_tag)
OAC_EXCEPTION_END()

/**
 * A syntax error was found while parsing the variable name.
 */
OAC_EXCEPTION_BEGIN(var_name_syntax_error, invalid_var_exception)
   OAC_EXCEPTION_FIELD(var_name_tag, variable_name::tag_type)
   OAC_EXCEPTION_MSG(
         "syntax error in variable name '%s': not a FSUIPC offset",
         var_name_tag)
OAC_EXCEPTION_END()

}

/**
 * Convert a variable ID to a FSUIPC offset.
 *
 * @param var_id The variable ID to be converted into FSUIPC offset
 * @throws fsuipc::invalid_var_group_error if var group is not fsuipc/offset
 * @throws fsuipc::var_name_syntax_error if the variable ID has a syntax error
 */
fsuipc_offset to_fsuipc_offset(
      const variable_id& var_id)
throw (fsuipc::invalid_var_group_error, fsuipc::var_name_syntax_error);

/**
 * Convert a FSUIPC valued offset into a variable value object.
 */
variable_value to_variable_value(
      const fsuipc_valued_offset& valued_offset);

/**
 * Convert a variable value into a FSUIPC offset value.
 */
fsuipc_offset_value to_fsuipc_offset_value(
      const variable_value& var_value);

/**
 * Subscription metadata. It is used internally by fsuipc_flight_vars to
 * implement its logic.
 */
class subscription_meta
{
public:

   subscription_meta(
         const variable_id& var_id,
         const flight_vars::var_update_handler& handler,
         const subscription_id& subs_id = make_subscription_id())
      : _var_id(var_id),
        _handler(handler),
        _subs_id(subs_id)
   {}

   bool operator == (const subscription_meta& subs) const
   { return _subs_id == subs._subs_id; }

   const variable_id& get_variable() const { return _var_id; }

   const flight_vars::var_update_handler& get_update_handler() const
   { return _handler; }

   const subscription_id get_subscription_id() const { return _subs_id; }

private:

   variable_id _var_id;
   flight_vars::var_update_handler _handler;
   subscription_id _subs_id;
};

/**
 * A in-memory database that maintains the relation among variables, offsets
 * and subscriptions. It is used internally by fsuipc_flight_vars to implement
 * its logic.
 */
class fsuipc_offset_db
{
public:

   /**
    * An unknown offset was specified.
    */
   OAC_EXCEPTION_BEGIN(no_such_offset_error, oac::exception)
      OAC_EXCEPTION_FIELD(offset_addr, fsuipc_offset_address)
      OAC_EXCEPTION_FIELD(offset_len, fsuipc_offset_length)
      OAC_EXCEPTION_MSG(
            "no such offset 0x%x:%d in FSUIPC offset database",
            offset_addr,
            offset_len)
   OAC_EXCEPTION_END()

   /**
    * An unknown subscription was specified.
    */
   OAC_EXCEPTION_BEGIN(no_such_subscription_error, oac::exception)
      OAC_EXCEPTION_FIELD(subscription, subscription_id)
      OAC_EXCEPTION_MSG(
            "no such subscription %d in FSUIPC offset database",
            subscription)
   OAC_EXCEPTION_END()

   typedef std::list<fsuipc_offset> offset_list;
   typedef std::list<subscription_meta> subscription_list;   

   subscription_meta create_subscription(
         const variable_id& var_id,
         const flight_vars::var_update_handler& callback)
   throw (fsuipc::invalid_var_group_error, fsuipc::var_name_syntax_error);

   void remove_subscription(
         const subscription_id& subs);

   const offset_list& get_all_offsets() const
   { return _known_offsets; }

   const subscription_list& get_subscriptions_for_offset(
         const fsuipc_offset& offset) const
   throw (no_such_offset_error);

   fsuipc_offset get_offset_for_subscription(
         const subscription_id& subs_id)
   throw (no_such_subscription_error);

private:

   typedef std::unordered_map<
         fsuipc_offset,
         subscription_list,
         fsuipc_offset::hash> offset_handler_map;

   offset_list _known_offsets;
   offset_handler_map _offset_handlers;

   void insert_known_offset(
         const fsuipc_offset& offset);

   void remove_known_offset(
         const fsuipc_offset& offset);

   subscription_meta insert_subscription(
         const fsuipc_offset& offset,
         const variable_id& var_id,
         const flight_vars::var_update_handler& callback);

   std::size_t remove_subscription(
         const fsuipc_offset& offset,
         const subscription_id& subs_id);
};

/**
 * A Flight Vars implementation which provides access to FSUIPC offsets.
 * For this implementation, each FSUIPC offset represents a variable. It may
 * be named as <offset>:<size>, where <offset> is the numerical representation
 * (decimal, hexadecimal...) of the offset and <size> is one of 1, 2 or 4,
 * indicating a value in bytes (BYTE, WORD and DWORD, respectively). E.g.,
 * 0x0354:2 means a WORD variable at offset 0x0354 (transponder code).
 */
template <typename FsuipcUserAdapter>
class fsuipc_flight_vars : public flight_vars, public logger_component
{
public:

   fsuipc_flight_vars()
      : logger_component("fsuipc_flight_vars"),
        _update_observer(
           std::bind(
              &fsuipc_flight_vars::on_offset_update,
              this,
              std::placeholders::_1))
   {
   }

   virtual subscription_id subscribe(
         const variable_id& var,
         const var_update_handler& handler)
   throw (unknown_variable_error)
   {
      try
      {
         auto subs = _db.create_subscription(var, handler);
         auto subs_id = subs.get_subscription_id();
         auto offset = _db.get_offset_for_subscription(subs_id);

         _update_observer.start_observing(offset);

         log(
            log_level::INFO,
            "Subscribing on %s with ID %d...",
            var_to_string(var),
            subs_id);

         return subs_id;
      }
      catch (fsuipc::invalid_var_exception& e)
      {
         OAC_THROW_EXCEPTION(unknown_variable_error()
               .with_var_group_tag(get_var_group(var).get_tag())
               .with_var_name_tag(get_var_name(var).get_tag())
               .with_cause(e));
      }
   }

   virtual void unsubscribe(const subscription_id& id)
   {
      try
      {
         auto offset = _db.get_offset_for_subscription(id);
         _db.remove_subscription(id);
         auto offsets = _db.get_all_offsets();
         if (std::find(offsets.begin(), offsets.end(), offset) == offsets.end())
            _update_observer.stop_observing(offset);
      }
      catch (const fsuipc_offset_db::no_such_subscription_error& e)
      {
         OAC_THROW_EXCEPTION(unknown_subscription_error()
               .with_subs_id(id)
               .with_cause(e));
      }
   }

   virtual void update(
         const subscription_id& subs_id,
         const variable_value& var_value)
   throw (unknown_variable_error, illegal_value_error)
   {
      try
      {
         auto offset = _db.get_offset_for_subscription(subs_id);
         update_offset(offset, var_value);
      }
      catch (fsuipc_offset_db::no_such_subscription_error&)
      {
         OAC_THROW_EXCEPTION(unknown_subscription_error()
               .with_subs_id(subs_id));
      }
      catch (variable_value::invalid_type_error&)
      {
         OAC_THROW_EXCEPTION(invalid_value_type_error()
               .with_subs_id(subs_id)
               .with_var_type(var_value.get_type()));
      }
   }

   const FsuipcUserAdapter& user_adapter() const
   { return _update_observer.get_client().user_adapter(); }

   FsuipcUserAdapter& user_adapter()
   { return _update_observer.get_client().user_adapter(); }

   void check_for_updates()
   { _update_observer.check_for_updates(); }

private:

   fsuipc_offset_db _db;
   fsuipc_update_observer<FsuipcUserAdapter> _update_observer;

   void on_offset_update(
         const fsuipc_valued_offset valued_offset)
   {
      for (auto& subs : _db.get_subscriptions_for_offset(valued_offset))
      {
         subs.get_update_handler()(
                  subs.get_variable(),
                  to_variable_value(valued_offset));
      }
   }

   void update_offset(
         const fsuipc_offset& offset,
         const variable_value& value)
   {
      std::list<fsuipc_valued_offset> updates(
               1,
               fsuipc_valued_offset(
                  offset,
                  to_fsuipc_offset_value(value)));
      _update_observer.get_client().update(updates);
   }
};

class local_fsuipc_flight_vars :
      public fsuipc_flight_vars<local_fsuipc_user_adapter>
{
public:

   static const variable_group VAR_GROUP;
};

typedef fsuipc_flight_vars<dummy_fsuipc_user_adapter> dummy_fsuipc_flight_vars;

}} // namespace oac::fv

#endif
