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

#ifndef OAC_FV_BROKER
#define OAC_FV_BROKER

#include <liboac/exception.h>

namespace oac { namespace fv { namespace mqtt {

OAC_DECL_ABSTRACT_EXCEPTION(broker_exception);

/**
 * An object able to run a MQTT broker.
 */
class broker_runner
{
public:

   virtual void run_broker() throw (broker_exception) = 0;

   virtual void shutdown_broker() throw (broker_exception) = 0;
};

}}} // namespace oac::fv::mqtt

#endif
