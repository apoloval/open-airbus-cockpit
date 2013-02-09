# Release Notes for Version 0.1

This is the Open Airbus Cockpit Software Distribution (OACSD). It contains 
libraries, plugins and applications aimed to support the integration between the 
hardware components of Open Airbus Cockpit (OAC) and the flight simulator
running on your PC. 

Unless OACSD has been designed for OAC hardware, the software components
may be used under any other platform and for any other purpose as long as you 
respect the terms of the GPL license under this software is distributed. 

## What's New In This Version

The version 0.1 is the first release of OACSD. 

* `+` Added Wilco Exporter, a plugin for FSX that maps the internal state of Wilco
Airbus series cockpits into FSUIPC offsets. 

* `+` Added liboac-commons, header files and static library providing common 
features required by the OACSD components. 

### Wilco Exporter

Wilco Exporter is a module for MS Flight Simulator X that exports the data of 
Wilco Airbus via FSUIPC. If you ever used FSUIPC Exporter, it's enough to say 
that Wilco Exporter is a open source, free-of-charge replacement.

Wilco Exporter is used as the primary interface to integrate the cockpit 
components of Open Airbus Cockpit with the simulator.

Wilco Exporter uses the same FSUIPC offsets as FSUIPC Exporter in order to 
guarantee the compatibility, so any client-software may use any of them.

For this release, Wilco Exporter is not fully functional. It presents the 
following restrictions. 

* It just supports a subset of the variables available in Wilco Airbus 
aircrafts. This support is the whole *EFIS Control Panel and Flight Control 
Unit* as described in the 
[OAC wiki](https://github.com/apoloval/open-airbus-cockpit/wiki/Wilco-Exporter).
* It just supports the A320 CFM model. 

In further releases, more variables and more Wilco aircraft models will 
be supported. 

### liboac-commons

For this release, liboac-commons provides C++ code for mapping abstract 
entities into FSUIPC. The most remarkable features for this release are:

* A set of classes that wraps FSUIPC making it easier to handle in C++ language.
* A set of classes that wraps SimConnect making it easir to handle in C++
language. 
* A mapping mechanism to import Airbus cockpit status from an abstract entity 
into FSUIPC offsets. This makes possible to map any kind of entity into FSUIPC 
by just implementing the appropriate classes that represent the cockpit
components. The mapping process is fully decoupled from that entity, so any
Airbus from any other manufacturer might be exported in the same FSUIPS offsets
with the lesser effort. 

## Usage

### Wilco Exporter

Wilco Exporter is a plugin for FSX. The installation process will copy it
into the directory of your choose, and will configure the `dll.xml` file for
the current user to load the plugin on FSX startup. 

It is likely that the first time you run FSX after installation, it will ask
if you want to run the plugin and trust on it for further runnings. It is
absolutely safe and recommended to answer yes to both questions. 

When FSX is running, the plugin will be running as well. It is able to detect
when one of the supported Wilco Airbus aircrafts has been loaded by the 
name of the aircraft. If you are loading a modified Wilco aircraft with a
custom livery, make sure that the name of the aircraft contains the following
substrings in order to being detected by Wilco Exporter.

* For A320 CFM, the name should contain the **A320 CFM** sub-string in its name. 

Once a Wilco Airbus aircraft is loaded (or when FSX is initiated and default
aircraft is a Wilco Airbus), the mapping process starts. You will find that
FSUIPC variables are updated as described in the 
[OAC wiki](https://github.com/apoloval/open-airbus-cockpit/wiki/Wilco-Exporter).

The plugin writes a log in C:\Windows\Temp\WilcoExporter.log. You may check
out this file if you suspect on any error. Any irregular behavior is reported
in this log file. You may use it for debugging, and it should be attached
to any bug report. 

### liboac-commons

If you are a programmer and want to, you may use liboac-commons in your
project as long as you respect the terms of the GPL license. If you are 
interested on using the library under proprietary software, please contact
me on apoloval@gmail.com. 

## Error Reporting

Programmers are humans, and humans are not perfect. Therefore, the software
we make is not perfect. It is possible that you find issues when using OACSD
components. If so, the best thing you might do is to report them using the
[issue tracking system](https://github.com/apoloval/open-airbus-cockpit/issues) 
of the project. Try to be as much precise as possible, and do not scrimp on
details. 

## Contact

If you have further questions or comments, please send an email to 
[apoloval@gmail.com](mailto:apoloval@gmail.com). 