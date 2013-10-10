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
!define MUI_WELCOMEPAGE_TITLE_3LINES
!define MUI_WELCOMEPAGE_TITLE "Welcome to Open Airbus Cockpit Software \
   Distribution ${CMAKE_PACKAGE_VERSION} Setup"
!insertmacro MUI_PAGE_WELCOME
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
Var /GLOBAL PluginConfFile
Var /GLOBAL ErrorCode
Var /GLOBAL PluginName
Var /GLOBAL RootElement
Var /GLOBAL AddonElement
Var /GLOBAL ParentElement

Function EnableFSXPlugin
   StrCpy $PluginConfFile "$APPDATA\Microsoft\FSX\dll.xml"
   CopyFiles "$PluginConfFile" "dll-before-OACSD-was-installed.xml"
   IfFileExists "$PluginConfFile" FileExist
      MessageBox MB_OK|MB_ICONSTOP "Cannot find DLL config file $PluginConfFile. \
         This might mean that FSX is not installed or was never executed \
         for current user. The installation may proceed, but Wilco Exporter \
         plugin shall be manually configured by editing $PluginConfFile file."
      Return
FileExist:
   ${xml::LoadFile} "$PluginConfFile" $ErrorCode
   IntCmp $ErrorCode 0 FileLoaded
      Abort "Cannot load plugin XML file: $PluginConfFile"
FileLoaded:
   Pop $PluginName
   ${xml::RootElement} $RootElement $ErrorCode
   StrCpy $AddonElement "/SimBase.Document/Launch.Addon/Name[text()='$PluginName']"
   ${xml::XPathNode} "$AddonElement" $ErrorCode
   IntCmp $ErrorCode -1 CreateEntry ModifyEntry ModifyEntry
ModifyEntry:
   ${xml::Parent} $RootElement $ErrorCode
   IntCmp $ErrorCode 0 +2
      Abort "Cannot obtain parent element of Name='$PluginName'"
   ${xml::FirstChildElement} "Path" $RootElement $ErrorCode
   IntCmp $ErrorCode 0 +2
      Abort "Cannot obtain child Path element for Launch.Addon"
   ${xml::SetText} "$INSTDIR\Modules\$PluginName.dll" $ErrorCode
   IntCmp $ErrorCode 0 +2
      Abort "Cannot set text for element Launch.Addon/Path"
   Goto SaveFile
CreateEntry:
   StrCpy $AddonElement \
   "<Launch.Addon>\
      <Name>$PluginName</Name>\
      <Disabled>False</Disabled>\
      <Path>$INSTDIR\Modules\$PluginName.dll</Path>\
   </Launch.Addon>"
   ${xml::CreateNode} "$AddonElement" $RootElement
   IntCmp $RootElement 0 +1 +2 +2
      Abort "Cannot create child element for plugin XML file:\n$AddonElement"
   ${xml::InsertEndChild} "$RootElement" $ErrorCode
   IntCmp $ErrorCode 0 +2
      Abort "Cannot add child element for plugin XML file:\n$AddonElement"
SaveFile:
   ${xml::SaveFile} "" $ErrorCode
   IntCmp $ErrorCode 0 +2
      Abort "Cannot save plugin XML file:\n$PluginConfFile"
   ${xml::Unload}
FunctionEnd

Function un.DisableFSXPlugin
   StrCpy $PluginConfFile "$APPDATA\Microsoft\FSX\dll.xml"
   CopyFiles "$PluginConfFile" "dll-before-OACSD-was-uninstalled.xml"
   IfFileExists "$PluginConfFile" FileExists
      MessageBox MB_OK|MB_ICONSTOP "Cannot find plugin file $PluginConfFile. \
         It might mean that FSX was uninstalled before OACSD or OACSD was \
         never configured for current user."
      Return
FileExists:
   ${xml::LoadFile} "$PluginConfFile" $ErrorCode
   IntCmp $ErrorCode 0 FileLoaded
      MessageBox MB_OK|MB_ICONSTOP "Cannot load plugin XML file $PluginConfFile. \
         Uninstaller cannot disable FSX plugin; you should do it manually \
         by editing $PluginConfFile file."
      Goto Unload
FileLoaded:
   Pop $PluginName
   ${xml::RootElement} $RootElement $ErrorCode
   StrCpy $AddonElement "/SimBase.Document/Launch.Addon/Name[text()='$PluginName']"
   ${xml::XPathNode} "$AddonElement" $ErrorCode
   IntCmp $ErrorCode 0 RemoveEntry
      MessageBox MB_OK "No entry found for $PluginName in $PluginConfFile. \
         This may be due to it was never installed or it was disabled by hand."
      Goto Unload
RemoveEntry:
   ${xml::Parent} $ParentElement $ErrorCode
   IntCmp $ErrorCode 0 NodeFound
      MessageBox MB_OK|MB_ICONSTOP "Cannot obtain parent element of \
         Name='$PluginName'. The plugin was not disabled, you'll have to \
         do it manually by editing $0 file."
      Goto Unload
NodeFound:
   ${xml::RemoveNode} $ErrorCode
   IntCmp $ErrorCode 0 SaveFile
      MessageBox MB_OK|MB_ICONSTOP "Cannot remove Launch.Addon node with \
         Name='$PluginName'. The plugin was not disabled, you'll have to \
         do it manually by editing $PluginConfFile file."
      Goto Unload
SaveFile:
   ${xml::SaveFile} "" $ErrorCode
   IntCmp $ErrorCode 0 +2
      Abort "Cannot save plugin XML file:\n$PluginConfFile"
Unload:
   ${xml::Unload}
FunctionEnd


###
# Sections
###
Section "-Write Uninstaller"
   SetOutPath "$INSTDIR"
   WriteUninstaller "$INSTDIR\Uninstall.exe"
SectionEnd

Section "FlightVars Plugin" SecFlightVars
   SetOutPath "$INSTDIR\Modules"
   File "${CMAKE_BINARY_DIR}\flightvars\FlightVars.dll"
   Push "FlightVars"
   Call EnableFSXPlugin
SectionEnd

Section "WilcoExporter Plugin" SecWilcoExporter
   SetOutPath "$INSTDIR\Modules"
   File "${CMAKE_BINARY_DIR}\wilco-exporter\WilcoExporter.dll"
   Push "WilcoExporter"
   Call EnableFSXPlugin
SectionEnd

Section "OAC Library" SecLibCommons
   SetOutPath "$INSTDIR\liboac\lib"
   File "${CMAKE_BINARY_DIR}\liboac\liboac.lib"
   SetOutPath "$INSTDIR\liboac\include\liboac"
   File "${CMAKE_SOURCE_DIR}\liboac\include\liboac\*"
SectionEnd

Section "un.FlightVars Plugin" 
   Push "FlightVars"
   Call un.DisableFSXPlugin
SectionEnd

Section "un.WilcoExporter Plugin" 
   Push "WilcoExporter"
   Call un.DisableFSXPlugin
SectionEnd

Section "Uninstall"
   RMDir /r "$INSTDIR"
   DeleteRegKey /ifempty HKCU "Software\OACSD"
SectionEnd

###
# Descriptions
###
LangString DESC_SecWilcoExporter ${LANG_ENGLISH} \
   "Wilco Exporter Plugin for FSX, gives access to Wilco Airbus cockpit via \
   FSUIPC offsets"
LangString DESC_SecFlightVars ${LANG_ENGLISH} \
      "FlightVars server plugin, grants access to simulation variables via \
      TCP/IP network."
LangString DESC_SecLibCommons ${LANG_ENGLISH} \
      "Header files and static library providing common basic features \
      used by OACSD components."

!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
!insertmacro MUI_DESCRIPTION_TEXT ${SecWilcoExporter} $(DESC_SecWilcoExporter)
!insertmacro MUI_DESCRIPTION_TEXT ${SecLibCommons} $(DESC_SecLibCommons)
!insertmacro MUI_FUNCTION_DESCRIPTION_END
