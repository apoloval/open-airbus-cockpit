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

require("lunit")

CommandGatewayIsTesting = true


------------------------------
-- Mocks for FSUIPC objects --
------------------------------

com = {
    NextContent = "",
    NextLength = 0
}

function com.open(port, baudrate, unknown)
    com.LastCall = { func = "open", baudrate = baudrate, unknown = unknown }
    return 0
end 

function com.read(handle, max, from, separator)
    com.LastCall = { func = "read", handle = handle,  max = max, 
        from = from, separator = separator 
    }
    return com.NextContent, com.NextLength
end

function com.close(handle)
    com.LastCall = { func = "close", handle = handle }
end

function com.OnRead(content, length)
    com.NextContent = content
    com.NextLength = length
end

ipc = {}

function ipc.log(line)
    -- print(line)
end

function ipc.writeLvar(lvar, value)
    ipc.LastCall = { func = "writeLvar", lvar = lvar, value = value }
end

function ipc.writeSB(offset, value)
    ipc.LastCall = { func = "writeSB", offset = offset, value = value }
end

function ipc.writeUB(offset, value)
    ipc.LastCall = { func = "writeUB", offset = offset, value = value }
end

function ipc.writeSW(offset, value)
    ipc.LastCall = { func = "writeSW", offset = offset, value = value }
end

function ipc.writeUW(offset, value)
    ipc.LastCall = { func = "writeUW", offset = offset, value = value }
end

function ipc.writeSD(offset, value)
    ipc.LastCall = { func = "writeSD", offset = offset, value = value }
end

function ipc.writeUD(offset, value)
    ipc.LastCall = { func = "writeUD", offset = offset, value = value }
end

require("OAC_CMDGW")

module("cmdgw_test", lunit.testcase, package.seeall)


----------------------------
-- Tests for line parsing --
----------------------------

function Test_ReadBeginLineMustReadFromFile()
    com.OnRead("BEGIN 1 ThatClient", 18)
    local ver, client = ReadBeginLine(123)

    assert_equal("read", com.LastCall.func)
    assert_equal(123, com.LastCall.handle)
    assert_equal(64, com.LastCall.max)
    assert_equal(-1, com.LastCall.from)
    assert_equal(10, com.LastCall.separator)
end

function Test_ReadBeginLineMustReturnCorrectData()
    com.OnRead("BEGIN 1 ThatClient", 18)
    local ver, client = ReadBeginLine(123)

    assert_equal('1', ver)
    assert_equal("ThatClient", client)
end

function Test_ReadBeginLineMustFailWhenInvalid()
    com.OnRead("BEGIN X Foobar", 18)
    local ver, client = ReadBeginLine(123)

    assert_equal(nil, ver)
    assert_equal(nil, client)
end

local function AssertWriteLvar(line, expectedLvar, expectedValue)
    local cmd = ParseCommand(line)
    assert_equal("WRITE_LVAR", cmd.action)
    assert_equal(expectedLvar, cmd.lvar)
    assert_equal(expectedValue, cmd.value)
end

function Test_ParseCommandMustParseWriteLVar()
    AssertWriteLvar("WRITE_LVAR MyVar 100", "MyVar", 100)
end

function Test_ParseCommandMustParseWriteLVarWithNegativeValue()
    AssertWriteLvar("WRITE_LVAR MyVar -100", "MyVar", -100)
end

function Test_ParseCommandMustParseWriteLVarWithUnderscoredName()
    AssertWriteLvar("WRITE_LVAR My_Var 100", "My_Var", 100)
end

local function AssertWriteOffset(line, expectedOffset, 
                                 expectedLength, expectedValue)
    local cmd = ParseCommand(line)
    assert_equal("WRITE_OFFSET", cmd.action)
    assert_equal(expectedOffset, cmd.offset)
    assert_equal(expectedLength, cmd.length)
    assert_equal(expectedValue, cmd.value)
end

function Test_ParseCommandMustParseWriteOffset()
    AssertWriteOffset("WRITE_OFFSET 123abc:ub 100", 0x123abc, "ub", 100)
end

function Test_ParseCommandMustParseWriteOffsetWithNegativeValue()
    AssertWriteOffset("WRITE_OFFSET 123abc:ub -100", 0x123abc, "ub", -100)
end

local function AssertObserveLVar(line, expectedLvar)
    local cmd = ParseCommand(line)
    assert_equal("OBS_LVAR", cmd.action)
    assert_equal(expectedLvar, cmd.lvar)
end

function Test_ParseCommandMustParseObserveLVar(line, expectedLvar)
    AssertObserveLVar("OBS_LVAR MyVar", "MyVar")
end

function Test_ParseCommandMustParseObserveLVarWithUnderscore(line, expectedLvar)
    AssertObserveLVar("OBS_LVAR My_Var", "My_Var")
end

local function AssertObserveOffset(line, expectedOffset, expectedLength)
    local cmd = ParseCommand(line)
    assert_equal("OBS_OFFSET", cmd.action)
    assert_equal(expectedOffset, cmd.offset)
    assert_equal(expectedLength, cmd.length)
end

function Test_ParseCommandMustParseObserveOffset(line, expectedLvar)
    AssertObserveOffset("OBS_OFFSET 123abc:ub", 0x123abc, "ub")
end


----------------------------------
-- Tests for command processing --
----------------------------------

function Test_ProcessWriteLVar()
    ProcessWriteLVar(123, "MyVar", 100)
    assert_equal("writeLvar", ipc.LastCall.func)
    assert_equal("MyVar", ipc.LastCall.lvar)
    assert_equal(100, ipc.LastCall.value)
end

local function AssertProcessWriteOffset(offset, len, value)
    ProcessWriteOffset(1234, offset, len, value)
    assert_equal(string.format("write%s", string.upper(len)), ipc.LastCall.func)
    assert_equal(offset, ipc.LastCall.offset)
    assert_equal(value, ipc.LastCall.value)
    ipc.LastCall = nil
end

function Test_ProcessWriteOffsetWithSB()
    AssertProcessWriteOffset(0x123abc, "SB", 100)
    AssertProcessWriteOffset(0x123abc, "sb", 100)
end

function Test_ProcessWriteOffsetWithUB()
    AssertProcessWriteOffset(0x123abc, "UB", 100)
    AssertProcessWriteOffset(0x123abc, "ub", 100)
end

function Test_ProcessWriteOffsetWithSW()
    AssertProcessWriteOffset(0x123abc, "SW", 100)
    AssertProcessWriteOffset(0x123abc, "sw", 100)
end

function Test_ProcessWriteOffsetWithUW()
    AssertProcessWriteOffset(0x123abc, "UW", 100)
    AssertProcessWriteOffset(0x123abc, "uw", 100)
end

function Test_ProcessWriteOffsetWithSD()
    AssertProcessWriteOffset(0x123abc, "SD", 100)
    AssertProcessWriteOffset(0x123abc, "sd", 100)
end

function Test_ProcessWriteOffsetWithUD()
    AssertProcessWriteOffset(0x123abc, "UD", 100)
    AssertProcessWriteOffset(0x123abc, "ud", 100)
end

function Test_ProcessWriteOffsetMustFailWithInvalidLength()
    ProcessWriteOffset(1234, 0x123abc, "verylong", 100)
    assert_equal(nil, ipc.LastCall)
    assert_equal("close", com.LastCall.func)
    assert_equal(1234, com.LastCall.handle)
end
