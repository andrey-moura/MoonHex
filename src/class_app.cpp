#include "class_app.hpp"

wxIMPLEMENT_APP(RomHexEditorApp);

bool RomHexEditorApp::OnInit()
{	
	MainFrame* mainFrame = new MainFrame();	
	mainFrame->Show();

	if (argc > 1)
	{
		wxArrayString args = argv.GetArguments();

		if (args[1] == "-f")
		{
			if (args.size() > 2)
			{				
				mainFrame->OpenFile(args[2]);				
			}
		}
	}	

	SetTopWindow(mainFrame);

	return true;
}