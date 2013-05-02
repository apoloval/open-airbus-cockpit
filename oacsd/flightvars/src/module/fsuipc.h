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

#include <Windows.h>

#include <liboac/fsuipc.h>
#include <liboac/lang-utils.h>
#include <liboac/simconn.h>

#include "api.h"

namespace oac { namespace fv {

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

   fsuipc_flight_vars(const ptr<buffer>& fsuipc = nullptr);

   virtual void subscribe(
         const variable_group& grp,
         const variable_name& name,
         const subscription& subs) throw (unknown_variable_error);

private:

   struct offset : tagged_element
   {
      variable_name var_name;
      DWORD address;
      DWORD length;

      offset(const variable_name& var_name) throw (unknown_variable_name_error);
      virtual bool isUpdated(double_buffer& buf);
      virtual variable_value read(double_buffer& buf);
   };

   typedef std::list<subscription> subscriptionList;
   typedef std::map<offset, subscriptionList> offsetsubscriptionsDict;

   offsetsubscriptionsDict _subscribers;
   simconnect_client _sc;
   ptr<buffer> _fsuipc;
   ptr<double_buffer> _buffer;

   /**
    * Check whether given var group corresponds to FSUIPC offset, and throw
    * a unknown_variable_group_error if not.
    */
   void check_group(const variable_group& grp) throw (unknown_variable_group_error);

   /**
    * Create a new subscription for given offset
    */
   void subscribe(const offset& offset, const subscription& subs);

   void notify_changes();

   void sync_offset(const offset& offset);
};

}} // namespace oac::fv

#endif
