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

#include <liboac/lang-utils.h>

#include "api.h"

namespace oac { namespace fv {

/**
 * Flight vars core object. The Flight Vars core object is the central
 * component of the Flight Vars module which registers the capabilities
 * of Flight Vars. It is implemented as a singleton so any other module
 * running in the simulator is able to interact with the core object in
 * order to register a new variable group master.
 */
class FlightVarsCore : public FlightVars
{
public:

   /**
    * Obtain the singleton instance of Flight Vars core object.
    */
   static Ptr<FlightVarsCore> instance();

   /**
    * Thrown in a group master registering attempt when there is already
    * a master registered for that group. It's complemented with:
    *
    *  - VariableGroupInfo, indicating the variable group which master
    *    is already present.
    */
   DECL_ERROR(GroupMasterAlreadyRegisteredError, InvalidInputError);

   virtual void subscribe(
         const VariableGroup& grp,
         const VariableName& name,
         const Subscription& subs) throw (UnknownVariableError);

   /**
    * Register a master for given variable group. If there is already a
    * master for given group, a GroupMasterAlreadyRegisteredError is thrown.
    */
   void registerGroupMaster(
         const VariableGroup& grp,
         const Ptr<FlightVars>& master)
   throw (GroupMasterAlreadyRegisteredError);

private:

   typedef std::map<VariableGroup::Tag, Ptr<FlightVars>> GroupMasterDict;

   GroupMasterDict _group_masters;

   inline FlightVarsCore() {}

};

}} // namespace oac::fv

#endif
