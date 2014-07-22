--
-- Open Airbus Cockpit - Log LVars
-- Copyright (c) 2014 Alvaro Polo
--
-- This Source Code Form is subject to the terms of the Mozilla Public
-- License, v. 2.0. If a copy of the MPL was not distributed with this
-- file, You can obtain one at http://mozilla.org/MPL/2.0/.
--
-- This module implements a command gateway functionality for Open Airbus
-- Cockpit devices. Essentially, it opens connections to serial ports
-- where OAC devices are found. Commands are read from these ports in order
-- to grant access to the devices to the internal state of the simulation.
--
-- For further information on OAC Command Gateway functionaly, please check
-- out http://github.com/apoloval/open-airbus-cockpit/tree/master/cmd-gw
--

-- Put a regular expression in `pattern` to filter the LVARs to be logged
local pattern = "AB_.*"

local names = {}
local values = {}

local i = 0
local numberOfNames = 0
while i < 65536 do
  name = ipc.getLvarName(i)
  if name ~= nil and name ~= names[i] then
    if string.match(name, pattern) ~= nil then
      names[i] = name
      values[i] = nil
      ipc.log(string.format("Scanning L:%s", name))
      numberOfNames = numberOfNames + 1
    end
  end
  i = i + 1
end

ipc.log(string.format(
  "%d LVARs found matching pattern %s", numberOfNames, pattern))

while 1 do
  for i, name in pairs(names) do
    old = values[i]
    new = ipc.readLvar(name)
    if old ~= new then
      if old == nil then
        ipc.log(string.format("L:%s is set to %d", name, new))
      else
        ipc.log(string.format("L:%s changes from %d to %d", name, old, new))
      end
      values[i] = new
    end
  end
end
