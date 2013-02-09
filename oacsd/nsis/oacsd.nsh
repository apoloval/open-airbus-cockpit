#
#  This file is part of Open Airbus Cockpit
#  Copyright (C) 2012 Alvaro Polo
#
#  Open Airbus Cockpit is free software: you can redistribute it and/or
#  modify it under the terms of the GNU General Public License as published
#  by the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  Open Airbus Cockpit is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with Open Airbus Cockpit.  If not, see <http://www.gnu.org/licenses/>.
#
#  ------------
#
#  This NSIS script generates the installer for OACSD.
#

!include "MUI2.nsh"
!include "XML.nsh"

!ifndef CMAKE_NSIS_OUTPUT
!define CMAKE_NSIS_OUTPUT setup.exe
!endif

!ifndef CMAKE_BINARY_DIR
!define CMAKE_BINARY_DIR
!endif

###
# General properties
###
InstallDir "$PROGRAMFILES\OACSD"
InstallDirRegKey HKCU "Software\OACSD" ""
Name "Open Airbus Cockpit Software Distribution"
OutFile "${CMAKE_NSIS_OUTPUT}"

###
# Pages
###
!insertmacro MUI_PAGE_LICENSE "license.txt"
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_UNPAGE_FINISH

###
# Languages
###
!insertmacro MUI_LANGUAGE "English"

###
# Functions
###
Function EnableFSXPlugin
   StrCpy $0 "$APPDATA\Microsoft\FSX\dll.xml"
   CopyFiles "$0" "dll-before-WilcoExporter-was-installed.xml"
   IfFileExists "$0" FileExist
      MessageBox MB_OK|MB_ICONSTOP "Cannot find DLL config file $0. \
         This might mean that FSX is not installed or was never executed \
         for current user. The installation may proceed, but Wilco Exporter \
         plugin shall be manually configured by editing $0 file."
      Return
FileExist:
   ${xml::LoadFile} "$0" $1
   IntCmp $1 0 FileLoaded
      Abort "Cannot load plugin XML file: $0"
FileLoaded:
   ${xml::RootElement} $2 $1
   StrCpy $3 "/SimBase.Document/Launch.Addon/Name[text()='Wilco Exporter']"
   ${xml::XPathNode} "$3" $1
   IntCmp $1 -1 CreateEntry ModifyEntry ModifyEntry
ModifyEntry:
   ${xml::Parent} $2 $1
   IntCmp $1 0 +2
      Abort "Cannot obtain parent element of Name='Wilco Exporter'"
   ${xml::FirstChildElement} "Path" $2 $1
   IntCmp $1 0 +2
      Abort "Cannot obtain child Path element for Launch.Addon"
   ${xml::SetText} "$INSTDIR\Modules\WilcoExporter.dll" $1
   IntCmp $1 0 +2
      Abort "Cannot set text for element Launch.Addon/Path"
   Goto SaveFile
CreateEntry:
   StrCpy $3 \
   "<Launch.Addon>\
      <Name>Wilco Exporter</Name>\
      <Disabled>False</Disabled>\
      <Path>$INSTDIR\Modules\WilcoExporter.dll</Path>\
   </Launch.Addon>"
   ${xml::CreateNode} "$3" $2
   IntCmp $2 0 +1 +2 +2
      Abort "Cannot create child element for plugin XML file:\n$3"
   ${xml::InsertEndChild} "$2" $1
   IntCmp $1 0 +2
      Abort "Cannot add child element for plugin XML file:\n$3"
SaveFile:
   ${xml::SaveFile} "" $1
   IntCmp $1 0 +2
      Abort "Cannot save plugin XML file:\n$0"
   ${xml::Unload}
FunctionEnd

Function un.DisableFSXPlugin
   StrCpy $0 "$APPDATA\Microsoft\FSX\dll.xml"
   CopyFiles "$0" "dll-before-WilcoExporter-was-uninstalled.xml"
   IfFileExists "$0" FileExists
      MessageBox MB_OK|MB_ICONSTOP "Cannot find plugin file $0. It might \
         mean that FSX was uninstalled before ACSD or Wilco Exporter was \
         never configured for current user."
      Return
FileExists:
   ${xml::LoadFile} "$0" $1
   IntCmp $1 0 FileLoaded
      MessageBox MB_OK|MB_ICONSTOP "Cannot load plugin XML file $0. \
         Uninstaller cannot disable FSX plugin; you should do it manually \
         by editing $0 file."
      Goto Unload
FileLoaded:
   ${xml::RootElement} $2 $1
   StrCpy $3 "/SimBase.Document/Launch.Addon/Name[text()='Wilco Exporter']"
   ${xml::XPathNode} "$3" $1
   IntCmp $1 0 RemoveEntry
      MessageBox MB_OK "No entry found for Wilco Exporter in $0. The plugin \
         was likely disabled by hand. "
      Goto Unload
RemoveEntry:
   ${xml::Parent} $2 $1
   IntCmp $1 0 NodeFound
      MessageBox MB_OK|MB_ICONSTOP "Cannot obtain parent element of \
         Name='Wilco Exporter'. The plugin was not disabled, you'll have to \
         do it manually by editing $0 file."
      Goto Unload
NodeFound:
   ${xml::RemoveNode} $1
   IntCmp $1 0 SaveFile
      MessageBox MB_OK|MB_ICONSTOP "Cannot remove Launch.Addon node with \
         Name='Wilco Exporter'. The plugin was not disabled, you'll have to \
         do it manually by editing $0 file."
      Goto Unload
SaveFile:
   ${xml::SaveFile} "" $1
   IntCmp $1 0 +2
      Abort "Cannot save plugin XML file:\n$0"
Unload:
   ${xml::Unload}
FunctionEnd


###
# Sections
###
Section "-Write Uninstaller"
   WriteUninstaller "$INSTDIR\Uninstall.exe"
SectionEnd

Section "WilcoExporter Plugin" SecWilcoExporter
   SetOutPath "$INSTDIR\Modules"
   File "${CMAKE_BINARY_DIR}\wilco-exporter\WilcoExporter.dll"
   Call EnableFSXPlugin
SectionEnd

Section "OAC Library - Commons" SecLibCommons
   SetOutPath "$INSTDIR\liboac-commons\lib"
   File "${CMAKE_BINARY_DIR}\liboac-commons\liboac-commons.lib"
   SetOutPath "$INSTDIR\liboac-commons\include\liboac"
   File "${CMAKE_SOURCE_DIR}\liboac-commons\include\liboac\*"
SectionEnd

Section "Uninstall"
   Call un.DisableFSXPlugin
   RMDir /r "$INSTDIR"
   DeleteRegKey /ifempty HKCU "Software\OACSD"
SectionEnd

###
# Descriptions
###
LangString DESC_SecWilcoExporter ${LANG_ENGLISH} \
   "Wilco Exporter Plugin for FSX, gives access to Wilco Airbus cockpit via\
   FSUIPC offsets"
LangString DESC_SecLibCommons ${LANG_ENGLISH} \
      "Open Airbus Cockpit Library - Commons. Header files and static library \
      providing common basic features used by OACSD components."

!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
!insertmacro MUI_DESCRIPTION_TEXT ${SecWilcoExporter} $(DESC_SecWilcoExporter)
!insertmacro MUI_DESCRIPTION_TEXT ${SecLibCommons} $(DESC_SecLibCommons)
!insertmacro MUI_FUNCTION_DESCRIPTION_END
