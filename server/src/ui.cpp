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

#include "ui.h"

#include "serial.h"

namespace oac { namespace server {

bool
TestFCUWindow::isActiveInstance()
{ return _numberOfInstances; }

TestFCUWindow::TestFCUWindow(TestFCUController* controller) : 
   wxFrame(NULL, WIN, wxT("Open Airbus Cockpit"), 
           wxDefaultPosition, wxDefaultSize,
           wxMINIMIZE_BOX | wxCLOSE_BOX | wxCAPTION),
   _controller(controller),
   _toggleSpeedModeState(false),
   _toggleHeadingModeState(false)
{
   wxBoxSizer* rootSizer = new wxBoxSizer(wxVERTICAL);
   
   /* Speed mode selection. */
   wxBoxSizer* speedModeSizer = new wxBoxSizer(wxHORIZONTAL);
   wxStaticText* speedModeText = 
      new wxStaticText(this, wxID_ANY, wxT("Selected speed"),
                       wxDefaultPosition, wxSize(100, -1),
                       wxALIGN_RIGHT);
   speedModeSizer->Add(speedModeText, 0, wxALL, 10);
   
   _selectedSpeedControl = new wxSpinCtrl(
         this, SPIN_SPEED, wxT("200"), 
         wxDefaultPosition, wxSize(50, -1));
   speedModeSizer->Add(_selectedSpeedControl, 0, wxEXPAND | wxALL, 10);
   _selectedSpeedControl->SetRange(50, 400);
   _selectedSpeedControl->Disable();
   
   this->_toggleSpeedModeButton = 
         new wxButton(this, TOGGLE_SPEED, wxT("Selected Mode"));		
   speedModeSizer->Add(this->_toggleSpeedModeButton, 0, 
                       wxEXPAND | wxALL, 10);
   
   rootSizer->Add(speedModeSizer, 0, wxALIGN_CENTER);
   
   /* Heading mode selection. */
   wxBoxSizer* headingModeSizer = new wxBoxSizer(wxHORIZONTAL);
   wxStaticText* headingModeText = new wxStaticText(
         this, wxID_ANY, wxT("Selected heading"), wxDefaultPosition, 
         wxSize(100, -1), wxALIGN_RIGHT);
   headingModeSizer->Add(headingModeText, 0, wxALL, 10);
   
   _selectedHeadingControl = new wxSpinCtrl(
         this, SPIN_HEADING, wxT("000"), wxDefaultPosition, wxSize(50, -1));
   headingModeSizer->Add(_selectedHeadingControl, 0, wxEXPAND | wxALL, 10);
   _selectedHeadingControl->SetRange(-1, 360);
   _selectedHeadingControl->Disable();
   
   this->_toggleHeadingModeButton = new wxButton(
         this, TOGGLE_HEADING, wxT("Selected Mode"));		
   headingModeSizer->Add(this->_toggleHeadingModeButton, 0, 
                         wxEXPAND | wxALL, 10);
   
   rootSizer->Add(headingModeSizer, 0, wxALIGN_CENTER);
		
   this->SetSizer(rootSizer);
   
   _numberOfInstances++;
}

TestFCUWindow::~TestFCUWindow()
{
   _numberOfInstances--;
   delete _controller;
}

unsigned int TestFCUWindow::_numberOfInstances = 0;
   
void
TestFCUWindow::onToggleSpeedMode(wxCommandEvent& event)
{
   _controller->fcu()->toggleSpeedMode();
   this->toggleMode(_toggleSpeedModeState, 
                    *_toggleSpeedModeButton, 
                    *_selectedSpeedControl);
}
	
void
TestFCUWindow::onToggleHeadingMode(wxCommandEvent& event)
{ 
   _controller->fcu()->toggleHeadingMode();
   this->toggleMode(_toggleHeadingModeState, 
                    *_toggleHeadingModeButton, 
                    *_selectedHeadingControl);
}

void
TestFCUWindow::onSpeedValueChanged(wxSpinEvent& event)
{
   _controller->fcu()->setSpeedValue(Speed(event.GetPosition()));
}
		
void
TestFCUWindow::onHeadingValueChanged(wxSpinEvent& event)
{
   int pos = event.GetPosition();
   if (pos == -1)
   {
      pos = 359;
      _selectedHeadingControl->SetValue(pos);
   }
   else if (pos == 360)
   {
      pos = 0;
      _selectedHeadingControl->SetValue(pos);
   }
   _controller->fcu()->setHeadingValue(Heading(pos));
}
		
void
TestFCUWindow::toggleMode(bool& stateFlag, wxButton& btn, wxSpinCtrl& ctrl)
{
   if (stateFlag)
   {
      btn.SetLabel(wxT("Selected Mode"));
      stateFlag = false;
      ctrl.Disable();
   }
   else
   {
      btn.SetLabel(wxT("Managed Mode"));
      stateFlag = true;
      ctrl.Enable();
   }
}

BEGIN_EVENT_TABLE(TestFCUWindow, wxFrame)
   EVT_BUTTON     (TOGGLE_SPEED,    TestFCUWindow::onToggleSpeedMode)
   EVT_BUTTON     (TOGGLE_HEADING,  TestFCUWindow::onToggleHeadingMode)
   EVT_SPINCTRL   (SPIN_SPEED,      TestFCUWindow::onSpeedValueChanged)
   EVT_SPINCTRL   (SPIN_HEADING,    TestFCUWindow::onHeadingValueChanged)
END_EVENT_TABLE()

TestFCUController::TestFCUController(wxApp* app, SerialDevice* serialDevice) :
   _app(app), _fcu(new TestFlightControlUnit()),
   _serialDevice(serialDevice), 
   _fcuManager(new FCUDeviceManager(_serialDevice, _fcu))
{
}

ConnectionWindow::ConnectionWindow(
      wxApp* app, ConnectionController* controller) :
   wxFrame(NULL, WIN, wxT("Open Airbus Cockpit - Connection List"),
           wxDefaultPosition, wxDefaultSize,
           wxMINIMIZE_BOX | wxCLOSE_BOX |wxCAPTION),
   _app(app), _controller(controller)
{
   wxBoxSizer* rootSizer = new wxBoxSizer(wxVERTICAL);
   
   wxBoxSizer* fcuSizer = initDeviceControls(
         _fcu, "Flight Control Unit", FCU_CONNECT, FCU_TEST);

   rootSizer->Add(fcuSizer, 0, wxALIGN_CENTER |wxALL, 20);
   
   this->SetSizer(rootSizer);
}

wxColour ConnectionWindow::DISCONNECTED_COLOUR(0x60, 0x00, 0x00);
wxColour ConnectionWindow::CONNECTED_COLOUR(0x00, 0x60, 0x00);   

wxBoxSizer*
ConnectionWindow::initDeviceControls(
      DeviceControls& ctrls, const wxString& title,
      int connectEvt, int testEvt)
{
   wxStaticBoxSizer* sizer = new wxStaticBoxSizer(
      wxHORIZONTAL, this, title);
   sizer->Add(new wxStaticText(this, wxID_ANY, wxT("Status: "),
                               wxDefaultPosition, wxSize(50, -1), 
                               wxALIGN_RIGHT),
              0, wxALL, 10);
   ctrls.statusText = new wxStaticText(this, wxID_ANY, wxT("disconnected"), 
         wxDefaultPosition, wxSize(75, -1), wxALIGN_LEFT);
   sizer->Add(ctrls.statusText, 0, wxALL, 10);
   
   ctrls.connectButton = new wxButton(this, connectEvt, wxT("Connect"));		
   sizer->Add(ctrls.connectButton, 0, wxEXPAND | wxALL, 10);
   
   ctrls.testButton = new wxButton(this, FCU_TEST, wxT("Test"));		
   sizer->Add(ctrls.testButton, 0, wxEXPAND | wxALL, 10);
   
   ctrls.testWindow = NULL;
   
   setStatus(ctrls, false);
   
   return sizer;
}

void
ConnectionWindow::onFCUConnectPressed(wxCommandEvent& event)
{
   if (!_controller->isFCUConnected())
      this->connectFCU();
   else
      this->disconnectFCU();
}

void
ConnectionWindow::onFCUTestPressed(wxCommandEvent& event)
{
   if (!TestFCUWindow::isActiveInstance())
      _fcu.testWindow = new TestFCUWindow(
            _controller->createTestFCUController());
   _app->SetTopWindow(_fcu.testWindow);
   _fcu.testWindow->Show();
}

void
ConnectionWindow::connectFCU()
{
   if (_controller->isFCUConnected())
      return;
   try
   {
      SerialDeviceInfo dev;
      if (this->selectSerialDevice(dev))
      {
         _controller->connectFCU(dev);
         
         setStatus(_fcu, true);
      }
   }
   catch (NotFoundException&)
   {
      wxMessageDialog* dialog = new wxMessageDialog(this, 
         wxT("No serial device was found. Please check that "
         "Arduino drivers are successfully installed"));
      dialog->ShowModal();
      delete dialog;
   }
}

void
ConnectionWindow::disconnectFCU()
{
   if (_controller->isFCUConnected())
   {
      _controller->disconnectFCU();
      setStatus(_fcu, false);
   }
}

bool
ConnectionWindow::selectSerialDevice(SerialDeviceInfo& dev)
throw (NotFoundException)
{
   SerialDeviceInfoArray devs;
   _controller->listSerialDevices(devs);
   if (devs.size() == 0)
      throw NotFoundException("no serial device was found");

   wxArrayString choices;
   for (unsigned int i = 0; i < devs.size(); i++)
      choices.Add(devs[i].name);
   wxSingleChoiceDialog* dialog = new wxSingleChoiceDialog(
         this, wxT("Please choice a USB/Serial port"), 
         wxT("Port selection"), choices);
   if (dialog->ShowModal() == wxID_CANCEL)
      return false;
   dev = devs[dialog->GetSelection()];
   return true;
}

void
ConnectionWindow::setStatus(DeviceControls& ctrls, bool connected)
{
   ctrls.statusText->SetLabel(connected ? "connected" : "disconnected");
   ctrls.statusText->SetForegroundColour(
      connected ? CONNECTED_COLOUR : DISCONNECTED_COLOUR);
   ctrls.connectButton->SetLabel(connected ? "Disconnect" : "Connect");
   ctrls.testButton->Enable(connected);
}

BEGIN_EVENT_TABLE(ConnectionWindow, wxFrame)
   EVT_BUTTON  (FCU_CONNECT,  ConnectionWindow::onFCUConnectPressed)
   EVT_BUTTON  (FCU_TEST,     ConnectionWindow::onFCUTestPressed)
END_EVENT_TABLE()



ConnectionController::ConnectionController(wxApp* app) : 
   _app(app),
   _serialDeviceManager(&SerialDeviceManager::getDefault()),
   _fcuSerialDevice(NULL)
{}

void
ConnectionController::listSerialDevices(SerialDeviceInfoArray& devs)
{ _serialDeviceManager->listSerialDevices(devs); }

void
ConnectionController::connectFCU(const SerialDeviceInfo& dev)
{
   if (_fcuSerialDevice)
      throw IllegalStateException("FCU already connected to serial device");
   _fcuSerialDevice = _serialDeviceManager->open(dev);
}

void
ConnectionController::disconnectFCU()
{
   if (!_fcuSerialDevice)
      throw IllegalStateException("FCU is not connected to any serial device");
   delete _fcuSerialDevice;
   _fcuSerialDevice = NULL;
}

TestFCUController*
ConnectionController::createTestFCUController()
{ 
   if (!_fcuSerialDevice)
      throw IllegalStateException("cannot create test FCU controller for "
                                  "disconnected serial device");
   return new TestFCUController(_app, _fcuSerialDevice);
}

}}; // namespace oac::server
