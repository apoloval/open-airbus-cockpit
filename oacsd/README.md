# Introduction

This is the Open Airbus Cockpit Software Distribution (OACSD). It contains 
libraries, plugins and applications to support the integration between OAC 
and the flight simulator following the design of Open Airbus Cockpit project. 

# Build

## Dependencies

Before building OACSD, you need to satisfy the following dependencies.

* Microsoft Windows XP or later. So far, OACSD doesn't work under any
other OS as Linux or Mac OS X. 
* MSVC (Microsoft Visual C++) 12.0 , which is distributed with MSVS (Microsoft
Visual Studio) 2013. Any version of MSVS 2013 is fine. If you have
problems to choose, likely MSVS 2013 Express for Windows Desktop is your
option. You may find download instructions 
[here](http://www.microsoft.com/visualstudio/eng/downloads).
* CMake 2.8 or later. You can download it from 
[here](http://www.cmake.org/cmake/resources/software.html).
* Flight Simulator X SDK. You may find it in your FSX installation DVD. 
* Boost libraries, you can download them from 
[here](http://www.boost.org/users/download/). You need to build it since
OACSD uses some of the compiled libraries. It is recommented to follow the
build instructions provided as part of Boost documentation.
* Nullsoft Scriptable Install System (NSIS). You can download it from
[here](http://nsis.sourceforge.net/Download). 
* NSIS XML Plugin. you can download it from
[here](http://nsis.sourceforge.net/XML_plug-in).

**Important note for Boost 1.54 and earlier versions**. There are some break
changes in MSVC 12.0 respect its previous versions on how it interprets C++11.
These break changes cause a build break in Boost 1.54 and earlier versions. If
you have this version (at the time this was written, 2013-09-02, that's the
latest version) you may have to [patch the code](
https://svn.boost.org/trac/boost/attachment/ticket/8750/for_vs2013.patch).


## Before building

OACSD is built with CMake. In order to work, it needs to know the location
of the dependencies shown above in your filesystem. In this context, we well
refer to these locations as:

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
   -DBoost_USE_STATIC_LIBS=ON
   -DBoost_COMPILER=-vc120
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

