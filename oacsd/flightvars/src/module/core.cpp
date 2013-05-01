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

#include "core.h"

namespace oac { namespace fv {

Ptr<FlightVarsCore>
FlightVarsCore::instance()
{
   static Ptr<FlightVarsCore> core(nullptr);
   if (!core)
      core = new FlightVarsCore();
   return core;
}

void
FlightVarsCore::subscribe(
      const VariableGroup& grp,
      const VariableName& name,
      const Subscription& subs)
throw (UnknownVariableError)
{
   auto entry = _group_masters.find(grp);
   if (entry == _group_masters.end())
      THROW_ERROR(UnknownVariableGroupError() << VariableGroupInfo(grp));
   entry->second->subscribe(grp, name, subs);
}

void
FlightVarsCore::registerGroupMaster(
      const VariableGroup& grp,
      const Ptr<FlightVars>& master)
throw (GroupMasterAlreadyRegisteredError)
{
   auto entry = _group_masters.find(grp);
   if (entry != _group_masters.end())
      THROW_ERROR(GroupMasterAlreadyRegisteredError() <<
                  VariableGroupInfo(grp));
   _group_masters[grp] = master;
}

}} // namespace oac::fv
