#include "dialog_preferences.hpp"

#include "frame_main.hpp"

void PreferencesDialog::CreateGUIControls()
{
	wxConfigBase* config = wxConfigBase::Get();
	size_t fontSize = config->ReadLong(L"/Preferences/FontPreferences/FontSize", 10);

	wxNotebook* tabControl = new wxNotebook(this, wxID_ANY);

	wxPanel* fontSettingsPanel = new wxPanel(tabControl);
	m_fontSizeSpin = new wxSpinCtrl(fontSettingsPanel, wxID_ANY);
	m_fontSizeSpin->SetMin(1);
	m_fontSizeSpin->SetValue(fontSize);

	wxBoxSizer* fontSizeSizer = new wxBoxSizer(wxHORIZONTAL);
	fontSizeSizer->Add(new wxStaticText(fontSettingsPanel, wxID_ANY, L"Font size"));
	fontSizeSizer->Add(m_fontSizeSpin);

	wxBoxSizer* fontSettingSizer = new wxBoxSizer(wxVERTICAL);
	fontSettingSizer->Add(fontSizeSizer);

	fontSettingsPanel->SetSizer(fontSettingSizer);

	tabControl->AddPage(fontSettingsPanel, "Font Settings");

	wxBoxSizer* rootSizer = new wxBoxSizer(wxVERTICAL);
	rootSizer->Add(tabControl, 1, wxEXPAND, 0);
	rootSizer->AddSpacer(4);
	rootSizer->Add(CreateButtonSizer(wxOK | wxCANCEL), 0, wxEXPAND, 0);

	SetSizer(rootSizer);

	Bind(wxEVT_BUTTON, [this](wxCommandEvent& event) { Commit(); event.Skip(); }, wxID_OK);
}

void PreferencesDialog::Commit()
{
	wxWindow* topWindow = wxTheApp->GetTopWindow();
	MainFrame* mainFrame = (MainFrame*)topWindow;

	if (m_fontSizeSpin) {
		size_t fontSize = m_fontSizeSpin->GetValue();
		if (fontSize) {
			wxConfigBase* config = wxConfigBase::Get();

			size_t oldFontSize = config->ReadLong(L"/Preferences/FontPreferences/FontSize", 0);

			if (!oldFontSize || fontSize != oldFontSize) {
				config->Write(L"/Preferences/FontPreferences/FontSize", fontSize);
				mainFrame->SetFontSize(fontSize);
			}
		}
	}
}