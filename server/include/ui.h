/*
 * This file is part of Open Airbus Cockpit
 * Copyright (C) 2012 Alvaro Polo
 *
 * Open Airbus Cockpit is free software: you can redistribute it and/or 
 * modify it under the terms of the GNU General Public License as published 
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * Open Airbus Cockpit is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Open Airbus Cockpit.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef OAC_SERVER_UI_H
#define OAC_SERVER_UI_H

#include <wx/wx.h>
#include <wx/spinctrl.h>

#include "exception.h"
#include "components.h"
#include "serial.h"
#include "test.h"
#include "devices.h"

namespace oac { namespace server {

class TestFCUController;
   
class TestFCUWindow : public wxFrame
{
public:

   enum IDs 
   {
      WIN          = 100,
      TOGGLE_SPEED,
      TOGGLE_HEADING,
      TOGGLE_VERTICAL_SPEED,
      SPIN_SPEED,
      SPIN_HEADING,
      SPIN_ALTITUDE,
      SPIN_VERTICAL_SPEED,
   };
   
   static bool isActiveInstance();
	
   TestFCUWindow(TestFCUController* controller);
   
   virtual ~TestFCUWindow();
   
private:

   static unsigned int _numberOfInstances;
   
   TestFCUController* _controller;

   wxButton* _toggleSpeedModeButton;
   bool _toggleSpeedModeState;
   wxSpinCtrl* _selectedSpeedControl;
   
   wxButton* _toggleHeadingModeButton;
   bool _toggleHeadingModeState;
   wxSpinCtrl* _selectedHeadingControl;
   
   wxSpinCtrl* _targetAltitudeControl;

   wxButton* _toggleVerticalSpeedModeButton;
   bool _toggleVerticalSpeedModeState;
   wxSpinCtrl* _selectedVerticalSpeedControl;
   
   void onToggleSpeedMode(wxCommandEvent& event);   
   void onToggleHeadingMode(wxCommandEvent& event);
   void onToggleVerticalSpeedMode(wxCommandEvent& event);
   void onSpeedValueChanged(wxSpinEvent& event);
   void onHeadingValueChanged(wxSpinEvent& event);
   void onTargetAltitudeChanged(wxSpinEvent& event);
   void onVerticalSpeedChanged(wxSpinEvent& event);
   
   void toggleMode(bool& stateFlag, wxButton& btn, wxSpinCtrl& ctrl);
   
   DECLARE_EVENT_TABLE()

};

class TestFCUController
{
public:

   TestFCUController(wxApp* app, SerialDevice* serialDevice);
   
   inline FlightControlUnit* fcu() 
   { return _fcu; }
   
private:

   wxApp* _app;
   FlightControlUnit* _fcu;
   SerialDevice* _serialDevice;
   FCUDeviceManager* _fcuManager;
   
};

class ConnectionController;

class ConnectionWindow : public wxFrame
{
public:

   enum IDs
   {
      WIN = 200, 
      FCU_CONNECT,
      FCU_TEST,
   };

   ConnectionWindow(wxApp* app, ConnectionController* controller);
   
private:

   static wxColour DISCONNECTED_COLOUR;
   static wxColour CONNECTED_COLOUR;

   wxApp* _app;
   ConnectionController* _controller;

   struct DeviceControls
   {
      wxStaticText*  statusText;
      wxButton*      connectButton;
      wxButton*      testButton;
      wxFrame*       testWindow;
   };
   
   DeviceControls _fcu;
   
   wxBoxSizer* initDeviceControls(DeviceControls& ctrls, const wxString& title,
                                  int connectEvt, int testEvt);

   void onFCUConnectPressed(wxCommandEvent& event);
   void onFCUTestPressed(wxCommandEvent& event);
   
   bool selectSerialDevice(SerialDeviceInfo& dev) throw (NotFoundException);
   
   void connectFCU();
   void disconnectFCU();
   
   static void setStatus(DeviceControls& ctrls, bool connected);
   
   DECLARE_EVENT_TABLE()
};

class ConnectionController
{
public:

   ConnectionController(wxApp* app);

   inline bool isFCUConnected() const
   { return _fcuSerialDevice; }
   
   inline SerialDevice* FCUSerialDevice() const
   { return _fcuSerialDevice; }
   
   void listSerialDevices(SerialDeviceInfoArray& devs);
   
   void connectFCU(const SerialDeviceInfo& dev);
   
   void disconnectFCU();
   
   TestFCUController* createTestFCUController();

private:

   wxApp* _app;
   SerialDeviceManager* _serialDeviceManager;
   SerialDevice* _fcuSerialDevice;

};

}}; // namespace oac::server

#endif
