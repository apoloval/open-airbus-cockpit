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
class FsuipcFlightVars : public FlightVars
{
public:

   static const VariableGroup VAR_GROUP;

   FsuipcFlightVars(const Ptr<Buffer>& fsuipc = nullptr);

   virtual void subscribe(
         const VariableGroup& grp,
         const VariableName& name,
         const Subscription& subs) throw (UnknownVariableError);

private:

   struct Offset : TaggedElement
   {
      VariableName var_name;
      DWORD address;
      DWORD length;

      Offset(const VariableName& var_name) throw (UnknownVariableNameError);
      virtual bool isUpdated(DoubleBuffer& buf);
      virtual VariableValue read(DoubleBuffer& buf);
   };

   typedef std::list<Subscription> SubscriptionList;
   typedef std::map<Offset, SubscriptionList> OffsetSubscriptionsDict;

   OffsetSubscriptionsDict _subscribers;
   SimConnectClient _sc;
   Ptr<Buffer> _fsuipc;
   Ptr<DoubleBuffer> _buffer;

   /**
    * Check whether given var group corresponds to FSUIPC offset, and throw
    * a UnknownVariableGroupError if not.
    */
   void checkGroup(const VariableGroup& grp) throw (UnknownVariableGroupError);

   /**
    * Parse variable name and convert into FSUIPC offset, or throw a
    * UnknownVariableNameError of given name doesn't match expected pattern.
    */
   Offset parseVarName(
         const VariableName& name) throw (UnknownVariableNameError);

   /**
    * Create a new subscription for given offset
    */
   void subscribe(const Offset& offset, const Subscription& subs);

   void notifyChanges();

   void syncOffset(const Offset& offset);
};

}} // namespace oac::fv

#endif
