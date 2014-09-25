---
layout: page
title: Command Gateway
group: navigation
permalink: software/command-gateway.html
section:
  - { title: "Software", url: "/software.html" }
---

{% include JB/setup %}

Command Gateway is a very simple software component aimed to support 
interfacing with the simulator. Here, interfacing means the ability for 
processes that run outside the simulator to interact with the simulated 
scene in terms of data manipulation. Command Gateway is the solution used by 
Open Airbus Cockpit hardware to read and write data from and to the systems 
being simulated in the main PC.

## How it works

We already said that Command Gateway is a very simple component, didn't we? 
It's so simple that it is just a script written in Lua language. If you don't
know anything about Lua, it's enough to say that it is a programming language
aimed to extend the functionality of a software program. And, in this case,
that software program is FSUIPC. 

FSUIPC is a Flight Simulator/Prepar3D addon mostly known as a extension of the 
interfacing capabilities of the simulator. It is widely used by addon vendors
to integrate their products with the sim. One of the most powerful but barely 
known features of FSUIPC is its scripting engine based in Lua language. It 
allows the user to write their own scripts to make use of the FSUIPC interface
to implement more sophisticated logic. And that's exactly what Command Gateway
does: it takes advantage of the FSUIPC interfaces by exporting them to the
outside world using protocols and channels that are not available in FSUIPC.

The operation mode of Command Gateway is as follows. As mentioned above, it is 
a script. When executed, it will try to scan all available serial ports of the
PC. For each port with a device connected to it, it will try to perform a 
short handshake to determine whether that's a Command Gateway compliant device. 
If so, it will expect to receive any of the following commands from the device:

* An FSUIPC offset observation request. The device has to indicate the size
and the address of the offset it wants to observe. It is used by the device to
indicate it wants to observe that offset and be notified every time it changes
its value. 
* A LVAR observation request. Similar to the previous one, but for LVARs, i.e.
data variables handled by the cockpit gauges of the simulated aircraft. 
* An FSUIPC offset write request. The device indicates the size and the 
address of the offset it wants to write to, along the new value for that 
offset.
* A LVAR write request. Similar to the previous one but for LVARs. 

As you may notice, there is no read operation in Command Gateway. This is one
of its design principles. The hardware devices are not really interested in 
reading data but in being notified when such data change. This technique 
allows your hardware to save computing resources by reducing the number of
interactions with the simulator. 

After all serial ports are scanned for compliant devices, they are able to
send one of the command messages described above. In addition, Command Gateway
will send messages to report changes in the FSUIPC offsets or LVARs only when 
they are produced. This process continues until the script is terminated or
killed. 

## Installation

In order to install Command Gatway, you will need:

* MS Flight Simulator X or Prepar3D installed in your system
* A registered copy of FSUIPC plugin installed and configured for your 
simulator. You can obtain it from the 
[FSUIPC website](http://www.schiratti.com/dowson.html).
* The Command Gateway script. You may find it in 
[GitHub repository](https://raw.githubusercontent.com/apoloval/open-airbus-cockpit/master/cmd-gw/OAC_CMDGW.lua).

<div class="warning">
Important note! FSUIPC scripting is only available for registered version of 
FSUIPC. You have to adquire a license for Command Gateway to work.
</div>

As mentioned above, the file OAC_CMDGW.lua contains the Command Gateway script 
aimed to be run in FSUIPC Lua scripting engine. To do so, just copy the file 
into `Modules/` subfolder of your simulator (e.g., 
`C:\Program Files (x86)\Microsoft Games\Flight Simulator X\Modules`). That's
all you need to install Command Gatway in your sim.

## Execution

You have two methods to run the Command Gateway script.

### Run on Startup

You can configure FSUIPC to run the script when the simulator starts. To do so, 
edit the file `Modules\FSUIPC4.ini` and add the following section at the bottom 
of the file.

<div class="code"><pre><code>
[Auto]
1=Lua OAC_CMDGW
</code></pre></div>

This will make the script to be run when FSUIPC is loaded.

### Run on Keystroke

This is the recommented method. Instead of run the script on simulator startup, 
the script will start when a keystroke is pressed. If there was any running 
instance of the script, it will be killed and replaced by a new instance. This 
allow the user to reset the script if a strange behavior occurs.

To configure a keystroke to run the script go to the FSUIPC config window by 
selecting the _Add-ons > FSUIPC_ option in the top bar. Then the tab 
_Key Presses_ is the appropriate one to configure keystrokes. Select the key 
combination of your choice (e.g, _CTRL + R_) and select _LUA OAC CMDGW_ in the 
control list. Press the OK botton below and that's all! The OAC Command Gateway 
will be launched each time you press the key stroke you have chosen.

## Troubleshooting

Sometimes things go wrong. If you experiment issues or problems with Command 
Gateway, it would be better off checking out its log file to have more 
details.

It is highly recommended to configure FSUIPC to log the Lua scripts in 
separate files. To do so, go to the _Logging_ tab in FSUIPC configuration
and mark the option _Log Lua plugins separately_. 

After executing the simulator with separated log files for each script, you'll
find the Command Gateway log file in `Modules\OAC_CMDGW.log` relative to your 
Flight Simulator/Prepar3D installation directory. You can open it with any 
text editor (Windows Notepad will do). There you will find messages lines of
the script telling what's doing. Please read these lines carefully and look
for any error message you may find. 

If after reading the log file you are not able to determine what's failing
or how to fix it, you can 
[report a bug in GitHub](https://github.com/apoloval/open-airbus-cockpit/issues/new)
or [contact by email](/contact.html).
 
## Command Gateway & Open Airbus Cockpit

As mentioned above, the purpose of Command Gateway in Open Airbus Cockpit is
to server as a interface for the hardware devices to the simulator. If you
have checked out the hardware section, you'll know that most of the hardware
devices are based on Arduino Platform. Each Arduino board uses its serial port
to communicate with Command Gateway and interact with the simulated scene
through it. 

<!-- TODO: link to the arduino libraries page. -->

In the set of Arduino libraries provided by Open Airbus Cockpit you will find
one that implements the Command Gateway protocol to make it easier to 
interface to the Command Gateway. Please have a look to the documentation for
further details.

## Protocol description

This information is only relevant if you plan to interact with Command Gateway
through its protocol from a custom device or addon. This is a specification
of the text protocol it uses to communicate with the device. 

Command Gateway uses a text protocol to communicate with the devices using
the serial port. The text must be encoded in UTF-8. The protocol comprises
the following messages.

* `BEGIN {version} {client ID}`. This is the handshake message sent by the
device once the serial port is open by the server. It indicates the version
number of the protocol and an arbitrary token to identify the client session.
* `WRITE_LVAR {lvar name} {value}`. A request sent by the client to write to
the indicated LVAR with the given numberic value. 
* `WRITE_OFFSET {address}:{size} {value}`. A request sent by the client to 
write to the offset indicated by given address and size, with the given value.
* `OBS_LVAR {lvar name}`. A request sent by the client to start observing a 
given LVAR.
* `OBS_OFFSET {address}:{size}`. A request sent by the client to start 
observing a given offset by its address and size. 
* `EVENT_LVAR {lvar name} {value}`. A event sent by the server to notify a 
change in the given LVAR value.
* `EVENT_OFFSET {address} {value}`. A event sent by the server to notify
a change in the offset at given address. 
