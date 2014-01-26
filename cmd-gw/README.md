# Open Airbus Cockpit Command Gateway

## Introduction

OAC Command Gateway is a module of OAC aimed to serve as a gateway of 
OACSP commands for the simulator. It is implemented in Lua language, and
executed using FSUIPC module. 

In order to run it, you will need a payware license of FSUIPC. 

## Installation

The file OAC_CommandGateway.lua contains a script aimed to be run in FSUIPC
Lua scripting engine. To do so, just copy the file into `Modules/` subfolder
of your simulator 
(e.g., `C:\Program Files (x86)\Microsoft Games\Flight Simulator X\Modules`). 
Then, you have two methods to run the script.

### Run on Startup

You can configure FSUIPC to run the script when the simulator starts. To do
so, edit the file `Modules\FSUIPC4.ini` and add the following section at the
bottom of the file.

```text
[Auto]
1=Lua OAC_CommandGateway
```

This will make the script to be run when FSUIPC is loaded. 

### Run on Keystroke

This is the recommented method. Instead of run the script on simulator
startup, the script will start when a keystroke is pressed. If there was any
running instance of the script, it will be killed and replaced by a new
instance. This allow the user to reset the script if a strange behavior occurs. 

To configure a keystroke to run the script go to the FSUIPC config window by
selecting the _Add-ons -> FSUIPC_ option in the top bar. Then the tab _Key 
Presses_ is the appropriate one to configure keystrokes. Select the key
combination you want (e.g, `CTRL + R`) and select _LUA OAC Command Gateway_
in the control list. Press the _OK_ botton below and that's all! The
OAC Command Gateway will be launched each time you press the key stroke
you have chosen. 
