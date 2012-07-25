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

	TestFCUWindow* _testFCUWindow;
   TestFCUController* _testFCUController;
   FlightControlUnit* _fcu;
   
};

IMPLEMENT_APP(ServerApp);

bool ServerApp::OnInit()
{
	_testFCUWindow = new TestFCUWindow();
   _testFCUController = new TestFCUController(_testFCUWindow);
   
	SetTopWindow(_testFCUWindow);
	_testFCUWindow->Show();

	return true;
}

}}; // namespace oac::server
