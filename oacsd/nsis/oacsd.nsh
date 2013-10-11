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

!include "fs-plugins.nsh"

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
Page custom SimSelectionPage SimSelectionPageLeave
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
Var /GLOBAL CheckBoxFSX
Var /GLOBAL CheckBoxPrepar3D

Function SimSelectionPage
   !insertmacro MUI_HEADER_TEXT "Simulator selection" "Select your simulation software"
   nsDialogs::Create 1018
   Pop $0
   ${NSD_CreateLabel} 0u 0u 100% 25u "This installer is able to configure \
      OACSD modules for your simulator. Please choose the simulators you want \
      OACSD modules to be configured for."
   Pop $0
   ${NSD_CreateCheckBox} 10u 30u 100% 10u "Microsoft Flight Simulator X"
   Pop $CheckBoxFSX
   ${NSD_CreateCheckBox} 10u 45u 100% 10u "Lockheed Martin Prepar3D"
   Pop $CheckBoxPrepar3D
   ${NSD_CreateLabel} 0u 65u 100% 25u "Importante notice: only available \
      simulators are listed above. If you choose none, OACSD will be installed \
      but the plugins will not be configured (you may do it manually when \
      installer finishes)."
   Pop $0

   ReadRegStr $0 HKCU "Software\Microsoft\Microsoft Games\Flight Simulator\10.0" "AppPath"
   ${If} $0 == ""
      ${NSD_AddStyle} $CheckBoxFSX ${WS_DISABLED}
   ${Else}
      ${NSD_Check} $CheckBoxFSX
   ${EndIf}
   ReadRegStr $0 HKCU "Software\LockheedMartin\Prepar3D" "AppPath"
   ${If} $0 == ""
      ${NSD_AddStyle} $CheckBoxPrepar3D ${WS_DISABLED}
   ${Else}
      ${NSD_Check} $CheckBoxPrepar3D
   ${EndIf}

   nsDialogs::Show
FunctionEnd

Function SimSelectionPageLeave
   ${NSD_GetState} $CheckBoxFSX $0
   ${If} $0 == ${BST_CHECKED}
      StrCpy $OAC_InstallPluginFSX yes
   ${Else}
      StrCpy $OAC_InstallPluginFSX no
   ${EndIf}

   ${NSD_GetState} $CheckBoxPrepar3D $0
   ${If} $0 == ${BST_CHECKED}
      StrCpy $OAC_InstallPluginPrepar3D yes
   ${Else}
      StrCpy $OAC_InstallPluginPrepar3D no
   ${EndIf}
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
   SetOutPath "$INSTDIR\Bin"
   File "${CMAKE_BINARY_DIR}\flightvars\FlightVarsExplorer.exe"

   ${OAC::EnablePlugin} FlightVars
SectionEnd

Section "WilcoExporter Plugin" SecWilcoExporter
   SetOutPath "$INSTDIR\Modules"
   File "${CMAKE_BINARY_DIR}\wilco-exporter\WilcoExporter.dll"

   ${OAC::EnablePlugin} WilcoExporter
SectionEnd

Section "OAC Library" SecLibCommons
   SetOutPath "$INSTDIR\liboac\lib"
   File "${CMAKE_BINARY_DIR}\liboac\liboac.lib"
   SetOutPath "$INSTDIR\liboac\include\liboac"
   File "${CMAKE_SOURCE_DIR}\liboac\include\liboac\*"
SectionEnd

Section "un.FlightVars Plugin" 
   ${OAC::DisablePlugin} "FlightVars"
SectionEnd

Section "un.WilcoExporter Plugin" 
   ${OAC::DisablePlugin} "WilcoExporter"
SectionEnd

Section "Uninstall"
   RMDir /r "$INSTDIR"
   DeleteRegKey /ifempty HKCU "Software\OACSD"
SectionEnd

###
# Descriptions
###
LangString DESC_SecFlightVars ${LANG_ENGLISH} \
      "FlightVars server plugin, grants access to simulation variables via \
      TCP/IP network."
LangString DESC_SecWilcoExporter ${LANG_ENGLISH} \
   "Wilco Exporter Plugin for FSX, gives access to Wilco Airbus cockpit via \
   FSUIPC offsets"
LangString DESC_SecLibCommons ${LANG_ENGLISH} \
      "Header files and static library providing common basic features \
      used by OACSD components."

!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
!insertmacro MUI_DESCRIPTION_TEXT ${SecFlightVars} $(DESC_SecFlightVars)
!insertmacro MUI_DESCRIPTION_TEXT ${SecWilcoExporter} $(DESC_SecWilcoExporter)
!insertmacro MUI_DESCRIPTION_TEXT ${SecLibCommons} $(DESC_SecLibCommons)
!insertmacro MUI_FUNCTION_DESCRIPTION_END
