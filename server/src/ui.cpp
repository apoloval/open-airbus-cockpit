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

namespace oac { namespace server {

TestFCUWindow::TestFCUWindow() : 
   wxFrame(NULL, -1, wxT("Open Airbus Cockpit"), 
           wxDefaultPosition, wxDefaultSize,
           wxMINIMIZE_BOX | wxCLOSE_BOX | wxCAPTION),
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
         this, wxID_ANY, wxT("200"), 
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
         this, wxID_ANY, wxT("000"), wxDefaultPosition, wxSize(50, -1));
   headingModeSizer->Add(_selectedHeadingControl, 0, wxEXPAND | wxALL, 10);
   _selectedHeadingControl->SetRange(0, 359);
   _selectedHeadingControl->Disable();
   
   this->_toggleHeadingModeButton = new wxButton(
         this, TOGGLE_HEADING, wxT("Selected Mode"));		
   headingModeSizer->Add(this->_toggleHeadingModeButton, 0, 
                         wxEXPAND | wxALL, 10);
   
   rootSizer->Add(headingModeSizer, 0, wxALIGN_CENTER);
		
   this->SetSizer(rootSizer);

   this->Connect(TOGGLE_SPEED, wxEVT_COMMAND_BUTTON_CLICKED,
                 wxCommandEventHandler(TestFCUWindow::onToggleSpeedMode));
   this->Connect(TOGGLE_HEADING, wxEVT_COMMAND_BUTTON_CLICKED,
                 wxCommandEventHandler(TestFCUWindow::onToggleHeadingMode));
}

void
TestFCUWindow::onToggleSpeedMode(wxCommandEvent& event)
{
   this->toggleMode(_toggleSpeedModeState, 
                    *_toggleSpeedModeButton, 
                    *_selectedSpeedControl);
}
	
void
TestFCUWindow::onToggleHeadingMode(wxCommandEvent& event)
{ 
   this->toggleMode(_toggleHeadingModeState, 
                    *_toggleHeadingModeButton, 
                    *_selectedHeadingControl);
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
   
}}; // namespace oac::server
