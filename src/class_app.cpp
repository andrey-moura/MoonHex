#include "class_app.hpp"

wxIMPLEMENT_APP(RomHexEditorApp);

bool RomHexEditorApp::OnInit()
{	
	wxFileName fn(wxStandardPaths::Get().GetExecutablePath());
	fn.SetExt(L"ini");

	wxFileConfig* configs = new wxFileConfig(wxEmptyString, wxEmptyString, fn.GetFullPath());
	configs->SetRecordDefaults();
	configs->EnableAutoSave();

	wxConfigBase::Set(configs);

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