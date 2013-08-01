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
#include <liboac/simconn.h>

#include "api.h"
#include "subscription.h"

namespace oac { namespace fv {

namespace fsuipc {

/**
 * The given var group is not FSUIPC offset. Contains:
 *   - variable_group_info, indicating the invalid variable group
 */
OAC_DECL_ERROR(invalid_var_group_error, invalid_input_error);

/**
 * A syntax error was found while parsing the variable name. Contains:
 *   - variable_name_info, indicating the erroneous variable name
 */
OAC_DECL_ERROR(var_name_syntax_error, invalid_input_error);

}

/**
 * FSUIPC offset metadata.  It is used internally by fsuipc_flight_vars to
 * implement its logic.
 */
class fsuipc_offset_meta
{
public:

   static std::size_t hash(const fsuipc_offset_meta& meta);

   fsuipc_offset_meta(
         const variable_id& var_id)
   throw (fsuipc::invalid_var_group_error, fsuipc::var_name_syntax_error);

   bool operator == (const fsuipc_offset_meta& meta) const
   { return _address == meta._address && _length == meta._length; }

   std::uint16_t address() const { return _address;  }

   std::size_t length() const { return _length;  }

private:

   std::uint16_t _address;
   std::size_t _length;
};

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
   OAC_DECL_ERROR(unknown_offset_error, invalid_input_error);

   /**
    * An unknown subscription was specified.
    */
   OAC_DECL_ERROR(unknown_subscription_error, invalid_input_error);

   typedef std::list<fsuipc_offset_meta> offset_list;
   typedef std::list<subscription_meta> subscription_list;

   fsuipc_offset_db() : _offset_handlers(512, fsuipc_offset_meta::hash) {}

   subscription_meta create_subscription(
         const variable_id& var_id,
         const flight_vars::var_update_handler& callback)
   throw (fsuipc::invalid_var_group_error, fsuipc::var_name_syntax_error);

   void remove_subscription(
         const subscription_id& subs);

   const offset_list& get_all_offsets() const
   { return _known_offsets; }

   const subscription_list& get_subscriptions_for_offset(
         const fsuipc_offset_meta& offset) const
   throw (unknown_offset_error);

   fsuipc_offset_meta get_offset_for_subscription(
         const subscription_id& subs_id)
   throw (unknown_subscription_error);

private:

   typedef std::unordered_map<
         fsuipc_offset_meta,
         subscription_list,
         std::size_t(*)(const fsuipc_offset_meta&)> offset_handler_map;

   offset_list _known_offsets;
   offset_handler_map _offset_handlers;

   void insert_known_offset(
         const fsuipc_offset_meta& offset);

   void remove_known_offset(
         const fsuipc_offset_meta& offset);

   subscription_meta insert_subscription(
         const fsuipc_offset_meta& offset,
         const variable_id& var_id,
         const flight_vars::var_update_handler& callback);

   std::size_t remove_subscription(
         const fsuipc_offset_meta& offset,
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
class fsuipc_flight_vars : public flight_vars
{
public:

   static const variable_group VAR_GROUP;

   fsuipc_flight_vars();

   virtual subscription_id subscribe(
         const variable_id& var,
         const var_update_handler& handler) throw (unknown_variable_error);

   virtual void unsubscribe(const subscription_id& id);

   virtual void update(
         const subscription_id& subs_id,
         const variable_value& var_value)
   throw (unknown_variable_error, illegal_value_error);

private:

   fsuipc_offset_db _db;
   simconnect_client _sc;
   ptr<local_fsuipc> _fsuipc;
   ptr<double_buffer<>> _buffer;

   void notify_changes();

   void sync_offset(
         const fsuipc_offset_meta& offset);

   bool is_offset_updated(
         const fsuipc_offset_meta& offset) const;

   variable_value read_offset(
         const fsuipc_offset_meta& offset) const;

   void write_offset(
         const fsuipc_offset_meta& offset,
         const variable_value& value);
};

}} // namespace oac::fv

#endif
