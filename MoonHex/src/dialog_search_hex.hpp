#pragma once

#include <wx/dialog.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/combobox.h>
#include <wx/button.h>

class SearchHexDialog : public wxDialog
{
public:
	SearchHexDialog();
private:
	static wxArrayString s_Offsets;
private:
	void OnOkButton(wxCommandEvent& event);
private:
	void CreateGUIControls();
};

