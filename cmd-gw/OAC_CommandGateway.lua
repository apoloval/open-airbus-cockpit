-- Module: OAC Command Gateway
-- Author: Alvaro Polo
--
-- This module implements a command gateway functionality for Open Airbus
-- Cockpit devices. Essentially, it opens connections to serial ports
-- where OAC devices are found. Commands are read from these ports in order
-- to grant access to the devices to the internal state of the simulation. 
--
-- For further information on OAC Command Gateway functionaly, please check
-- out http://github.com/apoloval/open-airbus-cockpit/

-- Process a WRITE_LVAR command from the device
--
--   lvar -> The LVAR to be written
--   value -> The value to be written to the LVAR
local function ProcessWriteLVar(lvar, value)
	ipc.log(string.format("  --- > Writting on %s value %s", lvar, value))
	ipc.writeLvar(lvar, tonumber(value, 10))
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
	ipc.log(string.format("Received %d bytes: %s", len, data))

	local cmd, lvar, val = string.match(data, "(WRITE_LVAR) ([%a%d_]+) (%d+)")
	if cmd then
		ProcessWriteLVar(lvar, val)
	else
		ipc.log(
			string.format("Cannot find a suitable command in line: %s", data));
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
		event.com(handle, 1024, -1, 10, "OnDataReceived")
	else
		ipc.log(string.format("Cannot open connection to serial port %s", port))
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

-- The entry point of OAC Command Gateway: connect to devices
ConnectDevices()
