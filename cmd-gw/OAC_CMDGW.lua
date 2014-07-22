--
-- Open Airbus Cockpit - Command Gateway
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


-- A table to store the observers of each event
local EventObservers = {}

-- Add a new observer handle for given event
--
--   event -> The event observed by the given handle
--   handle -> The handle of the event observer
function EventObservers:insert(event, handle)
    local observers = self[event]
    if observers == nil then
        observers = {}
    end
    table.insert(observers, handle)
    EventObservers[event] = observers
end

-- Execute the given function for each observer of given event
--
--   event -> The event whose handlers will be passed to func
--   func -> A function(handle) to be invoked for each handle observing
--           the event
function EventObservers:foreach(event, func)
    local observers = self[event]
    if observers ~= nil then
        for i, obs in ipairs(observers) do
            func(obs)
        end
    else
        ipc.log(string.format("no handle found for event %s", event))
    end
end



-- Process a WRITE_LVAR command from the device
--
--   handle -> The serial port handle
--   lvar -> The LVAR to be written
--   value -> The value to be written to the LVAR
function ProcessWriteLVar(handle, lvar, value)
    ipc.log(string.format("WRITE_LVAR '%s' with value %d", lvar, value))
    ipc.writeLvar(lvar, value)
end

-- Process a WRITE_OFFSET command from the device.
--
--   handle -> The serial port handle
--   offset -> The OFFSET to be read
--   len -> The length of the offset (SB, UB, SW, UW, SD, UD)
--   value -> The value to be written to the offset
function ProcessWriteOffset(handle, offset, len, value)
    local len = string.upper(len)
    ipc.log(string.format(
        "WRITE_OFFSET 0x%x:%s with value %s", offset, len, value))
    if len == "SB" then
        ipc.writeSB(offset, value)
    elseif len == "UB" then
        ipc.writeUB(offset, value)
    elseif len == "SW" then
        ipc.writeSW(offset, value)
    elseif len == "UW" then
        ipc.writeUW(offset, value)
    elseif len == "SD" then
        ipc.writeSD(offset, value)
    elseif len == "UD" then
        ipc.writeUD(offset, value)
    else
        ipc.log(string.format(
            "Protocol violation: invalid offset length %s", len))
        ipc.log("Closing connection to remote device")
        com.close(handle)
    end
end

-- Process a OBS_LVAR command from the device
--
--   handle -> The serial port handle
--   lvar -> The LVAR to observe
local function ProcessObserveLVar(handle, lvar)
    ipc.log(string.format("OBS_LVAR '%s'", lvar))
    EventObservers:insert(lvar, handle)
    event.Lvar(lvar, 100, "OnLVarModified")
end

-- Process a OBS_OFFSET command from the device
--
--   handle -> The serial port handle
--   lvar -> The offset to observe
--   len -> The length of the offset (SB, UB, SW, UW, SD, UD)
local function ProcessObserveOffset(handle, offset, len)
    ipc.log(string.format("OBS_OFFSET %x:%s", offset, len))
    EventObservers:insert(offset, handle)
    event.offset(offset, len, "OnOffsetModified")
end

-- Read the begin line from the device.
--
--   handle -> The serial port handle.
--   return -> A (number, string) tuple indicating the protocol version
--             and the client name, respectively, or nil if begin line read
--             fails or contains invalid data. 
function ReadBeginLine(handle)
    local line, len
    repeat
        line, len = com.read(handle, 64, -1, 10) -- 10 == '\n'
    until len ~= 0
    local ver, client = string.match(line, "BEGIN (%d+) ([%a%d_ ]+)")
    if client ~= nil then
        return ver, client
    else
        ipc.log(string.format("Error while processing begin line: %s", line))
        return nil, nil
    end
end

function ParseCommand(data)
    if data == nil or string.len(data) == 0 then
        ipc.log("cannot parse command: null or empty data")
    end
    local cmd, lvar, val =
        string.match(data, "(WRITE_LVAR) ([%a%d_]+) (-?%d+)")
    if cmd then
        return {
            action = cmd,
            lvar = lvar,
            value = tonumber(val)
        }
    end

    local cmd, offset, len, val =
        string.match(data, "(WRITE_OFFSET) (%x+):(%a+) (-?%d+)")
    if cmd then
        return {
            action = cmd,
            offset = tonumber(offset, 16),
            length = len,
            value = tonumber(val)
        }
    end

    local cmd, lvar =
        string.match(data, "(OBS_LVAR) ([%a%d_]+)")
    if cmd then
        return {
            action = cmd,
            lvar = lvar
        }
    end

    local cmd, offset, len =
        string.match(data, "(OBS_OFFSET) (%x+):(%a+)")
    if cmd then
        return {
            action = cmd,
            offset = tonumber(offset, 16),
            length = len
        }
    end

    ipc.log(
        string.format("Cannot find a suitable command in line: %s", data))
    return nil
end

-- Callback function for LVAR modified event.
--
--   lvar -> The LVAR that was modified
--   value -> The new value of the LVAR
function OnLVarModified(lvar, value)
    local cmd = string.format("EVENT_LVAR %s %d", lvar, value)
    ipc.log(cmd)
    EventObservers:foreach(lvar,
        function (handle)
            com.write(handle, string.format("%s\n", cmd))
        end
    )
end


-- Callback function for FSUIPC offset modified event.
--
--   offset -> The offset that was modified
--   value -> The new value of the offset
function OnOffsetModified(offset, value)
    local cmd = string.format("EVENT_OFFSET %x %d", offset, value)
    ipc.log(cmd)
    EventObservers:foreach(offset,
        function (handle)
            com.write(handle, string.format("%s\n", cmd))
        end
    )
end

-- Callback function for data reception from serial port.
--
-- It processes the incoming data in order to match a known OACSP command.
-- If match, the corresponding treatment function is invoked.
--
--   handle -> The serial port handle
--   data -> The data read (as a string)
--   len -> The length of the data read
function OnDataReceived(handle, data, len)
    if len > 0 then
        local cmd = ParseCommand(data)

        if cmd == nil then
            ipc.log(string.format(
                "Cannot find a suitable command in line: %s", data))
        end

        if cmd.action == "WRITE_LVAR" then
            ProcessWriteLVar(handle, cmd.lvar, cmd.value)
        elseif cmd.action == "WRITE_OFFSET" then
            ProcessWriteOffset(handle, cmd.offset, cmd.length, cmd.value)
        elseif cmd.action == "OBS_LVAR" then
            ProcessObserveLVar(handle, cmd.lvar)
        elseif cmd.action == "OBS_OFFSET" then
            ProcessObserveOffset(handle, cmd.offset, cmd.length)
        else
            ipc.log(string.format(
                "Unknow parsed command action %s", cmd.action))
        end
    end
end

-- Open a new connection to given serial port.
--
-- This function tries to open a new connection to the given serial port.
-- If success, the resulting port handle is configured with OnDataReceived
-- callback to process any input data.
--
--   port -> The serial port to open a new connection (e.g., COM1, COM3...)
local function OpenConnection(port)
    local handle = com.open(port, 9600, 0)
    if handle ~= 0 then
        local ver, client = ReadBeginLine(handle)
        if client ~= nil then
            ipc.log(string.format(
                "Opening connection on port %s: '%s' with protocol version %d",
                port, client, ver))
            event.com(handle, 1024, -1, 10, "OnDataReceived")
        else
            ipc.log(string.format("Connection closed to %s", port))
            com.close(handle)
        end
    end
end

-- Try to connect any device attached to any serial port.
--
-- This function tries to connect all the devices that might be connected
-- to any serial port. It does so by brote force, trying to connect to ports
-- from COM3 to COM32 using the OpenConnection() function.
local function ConnectDevices()
    for i=3,32 do
        local port = string.format("COM%d", i)
        OpenConnection(port)
    end
end

if CommandGatewayIsTesting ~= true then
    -- The entry point of OAC Command Gateway: connect to devices
    ConnectDevices()
end
