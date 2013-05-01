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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Open Airbus Cockpit. If not, see <http://www.gnu.org/licenses/>.
 */

#define BOOST_AUTO_TEST_MAIN
#include <boost/test/auto_unit_test.hpp>

#include <liboac/buffer.h>

#include "fsuipc.h"

using namespace oac;
using namespace oac::fv;

BOOST_AUTO_TEST_CASE(ShouldRegister)
{
   Ptr<FlightVars> fsuipc = new FsuipcFlightVars(new FixedBuffer(0xffff));
   fsuipc->subscribe(
            VariableGroup("fsuipc/offset"),
            VariableName("0x200:2"),
            [](const VariableGroup&, const VariableName&, const VariableValue&) {});
}
