#pragma once

#include <wx/wx.h>
#include <wx/notebook.h>
#include <wx/spinctrl.h>
#include <wx/confbase.h>

class PreferencesDialog : public wxDialog
{
public:
	PreferencesDialog(wxWindow* parent)
		: wxDialog(parent, wxID_ANY, L"Preferences")
	{
		CreateGUIControls();
	}
private:
	void Commit();
private:
	void CreateGUIControls();
private:
	wxSpinCtrl* m_fontSizeSpin = nullptr;

};