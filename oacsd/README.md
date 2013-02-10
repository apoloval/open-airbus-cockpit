# Introduction

This is the Open Airbus Cockpit Software Distribution (OACSD). It contains 
libraries, plugins and applications to support the integration between OAC 
and the flight simulator following the design of Open Airbus Cockpit project. 

# Build

## Dependencies

Before building OACSD, you need to satisfy the following dependencies.

* Microsoft Windows XP or later. So far, OACSD doesn't work under any
other OS as Linux or Mac OS X. 
* Microsoft Visual C++ 2011 (MSVC), which is distributed with Microsoft
Visual Studio 2012 (MSVS). Any version of MSVS 2012 is fine. If you have
problems to choose, likely MSVS 2012 Express for Windows Desktop is your 
option. You may find download instructions 
[here](http://www.microsoft.com/visualstudio/eng/downloads).
* CMake 2.8 or later. You can download it from 
[here](http://www.cmake.org/cmake/resources/software.html).
* Flight Simulator X SDK. You may find it in your FSX installation DVD. 
* FSUIPC SDK, you can download it from 
[here](http://fsuipc.simflight.com/beta/FSUIPC_SDK.zip).
* Boost libraries, you can download them from 
[here](http://www.boost.org/users/download/). You need to build it since
OACSD uses some of the compiled libraries. It is recommented to follow the
build instructions provided as part of Boost documentation.
* Nullsoft Scriptable Install System (NSIS). You can download it from
[here](http://nsis.sourceforge.net/Download). 
* NSIS XML Plugin. you can download it from
[here](http://nsis.sourceforge.net/XML_plug-in).

## Building FSUIPC Internal

OACSD uses a special FSUIPC library that makes possible to communicate
FSUIPC and OAC plugins using direct memory access instead of the more heavy
IPC system calls. You may find that library in both source and binary forms
in FSUIPC SDK. 

Unfortunatelly, the binary form in SDK versioned as February 10th 2012 is 
linked against Windows libraries that are not compatible with MSVC 2011. 
So you need to build them again. And that's not all. The project file that
comes with the source code has a building option that makes the resulting
library to fail when connecting to the simulator. So please **read carefully
the following steps**. 

* Go to the `Library for FS Internal Users\FSUIPC_Internal` under your 
FSUIPC SDK folder. There you will find `FSUIPC_Internal.sln`, the solution 
file for MSVS. Open it.
* Perhaps MSVS will ask if you want to convert the solution to MSVS 2012 format.
In that case, answer *yes*. 
* With the solution open in MSVS, your will find a panel on the top right
corner of the Window listing the tree of artifacts of the solution. The 
fist element of that tree under root node is `FSUIPC_Internal` project.
Double-click on it and choose *Properties*. 
* In the properties Window, go to *Configuration Properties* -> *General* ->
*Project Default Values* and set the option *Character Set* to *Not Set*.
Accept the changes.
* Now you can build by pressing F7 key. 

After successful building, there should be a `Debug` subfolder, which contains
`ModuleUser.lib` and `FSUIPC_Internal.h` files. 

## Before building

OACSD is built with CMake. In order to work, it needs to know the location
of the dependencies shown above in your filesystem. In this context, we well
refer to these locations as:

* `<fsuipc_internal_include>`, the folder containing the file 
`FSUIPC_Internal.h`.  If you followed the steps described above, that's 
the `Debug` subfolder. 
* `<fsuipc_internal_library>`, the file `ModuleUser.lib` we obtained while
building FSUIPC Internal. 
* `<boost_root>`, the folder containing Boost Libraries distribution. Where
you unpacked the corresponding Boost ZIP file. 
* `<simconnect_root>`, the folder where SimConnect is installed. Its default
location is 
`C:\Program Files (x86)\Microsoft Games\Microsoft Flight Simulator X SDK\SDK\Core Utilities Kit\SimConnect SDK`. 
* `<nsis_root> the folder where NSIS was installed. Its default location is 
`C:\Program Files (x86)\NSIS`. 
* `<oacsd_root>` the folder containing OACSD. I.e., the folder where you 
found *this very* file. 

## Building

These instructions assume that you are building using the Windows command line
(cmd.exe). You may replicate the analogous steps to make it work under your 
favorite IDE or generator. 

First of all, you have to setup the correct paths for CMake to detect where
MSVC is installed. You have several alternatives to do that.
* Execute a regular console and run the batch file `vcvarsall.bat` located in
the `VC` directory in your MSVS installation. 
* Run a console from the Start menu by choosing the *Developer Command Prompt*
in the MSVS menu. 

With the correct environment set, let's create a folder where to store the 
built files. It may be located anywhere in your filesystem, but it is 
recommended to place it under `<oacsd_root>`. Let's name `<build_root>` to that 
directory. Go to that directory and use CMake to build OACSD invoking as 
follows.

```bash
mkdir <build_root>
cd <build_root>
cmake 
   -DBOOST_ROOT=<boost_root>
   -DFSUIPC_INTERNAL_INCLUDE_DIR=<fsuipc_internal_include>
   -DFSUIPC_INTERNAL_LIBRARY=<fsuipc_internal_library>
   -G "NMake Makefiles"
   <oacsd_root>
```

As you may notice, it is not necessary to indicate where SimConnect or NSIS
are, as long as you installed them in their default location. Otherwise, 
the cmake invocation above will fail. You may set the concrete location for
both tools by the following cache entries.

```bash
   -DSIM_CONNECT_INCLUDE_DIR=<simconnect_root>/inc
   -DSIM_CONNECT_LIBRARY=<simconnect_root>/lib
   -DMAKENSIS_BIN=<nsis_root>/makensis.exe
```

If everything goes well (and it should), you'll have NMake makefiles in
`<build_root>` ready to build OACSD. Just execute name indicating the 
targets you are interested. 

```bash
REM To make all libraries, programs and plugins
nmake all
REM To make the Windows installer
nmake nsis
```

After nmake do its job, you'll have the libraries, programs, plugins and 
installer build in `<build_root>`. 

