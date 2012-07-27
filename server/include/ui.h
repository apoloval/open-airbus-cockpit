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

namespace oac { namespace server {

class ConnectionController;

class ConnectionWindow : public wxFrame
{
public:

   enum IDs
   {
      FCU_CONNECT,
   };

   ConnectionWindow(ConnectionController* controller);
   
private:

   ConnectionController*   _controller;

   wxStaticText*           _fcuStatusText;
   wxButton*               _fcuConnectButton;
   
   void onFCUConnectPressed(wxCommandEvent& event);
   
   bool selectSerialDevice(SerialDeviceInfo& dev) throw (NotFoundException);
   
   void connectFCU();

};

class ConnectionController
{
public:

   ConnectionController();

   inline bool isFCUConnected() const
   { return _fcuConnected; }
   
   void listSerialDevices(SerialDeviceInfoArray& devs);

private:

   bool _fcuConnected;
   SerialDeviceManager* _serialDeviceManager;

};

class TestFCUWindow : public wxFrame
{
public:

   enum IDs 
   {
      TOGGLE_SPEED = 100,
      TOGGLE_HEADING,
   };
	
   TestFCUWindow();
   
private:

   wxButton* _toggleSpeedModeButton;
   bool _toggleSpeedModeState;
   wxSpinCtrl* _selectedSpeedControl;
   
   wxButton* _toggleHeadingModeButton;
   bool _toggleHeadingModeState;
   wxSpinCtrl* _selectedHeadingControl;

   void onToggleSpeedMode(wxCommandEvent& event);
   
   void onToggleHeadingMode(wxCommandEvent& event);
   
   void toggleMode(bool& stateFlag, wxButton& btn, wxSpinCtrl& ctrl);

};

class TestFCUController : public wxEvtHandler
{
public:

   TestFCUController(TestFCUWindow* window);

private:

	TestFCUWindow* _window;
   FlightControlUnit* _fcu;
   
   void onSpeedModeToggled(
         const FlightControlUnit::EventSpeedModeToggled& ev);
   
   void onSpeedValueChanged(
         const FlightControlUnit::EventSpeedValueChanged& ev);
   
   void onHeadingModeToggled(
         const FlightControlUnit::EventHeadingModeToggled& ev);
   
   void onHeadingValueChanged(
      const FlightControlUnit::EventHeadingValueChanged& ev);
      
   void onTargetAltitudeChanged(
      const FlightControlUnit::EventTargetAltitudeValueChanged& ev);
   
};

}}; // namespace oac::server

#endif
