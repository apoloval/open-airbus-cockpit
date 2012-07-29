#include <wx/wx.h>

#include "ui.h"

namespace oac { namespace server {

class ServerApp : public wxApp
{
public:
	// function called at the application initialization
	virtual bool OnInit();

	// event handler for button click
	void OnClick(wxCommandEvent& event)
	{
		GetTopWindow()->Close();
	}		

private:

   ConnectionWindow* _connectionWindow;
	TestFCUWindow* _testFCUWindow;
   TestFCUController* _testFCUController;
   FlightControlUnit* _fcu;
   
};

IMPLEMENT_APP(ServerApp);

bool ServerApp::OnInit()
{
   _connectionWindow = new ConnectionWindow(this, new ConnectionController(this));
   this->SetTopWindow(_connectionWindow);
   _connectionWindow->Show();

	return true;
}

}}; // namespace oac::server
