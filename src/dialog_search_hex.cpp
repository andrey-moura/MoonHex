#include "dialog_search_hex.hpp"

wxArrayString SearchHexDialog::s_Offsets;

SearchHexDialog::SearchHexDialog() : wxDialog(nullptr, wxID_ANY, "Search hex values")
{
	CreateGUIControls();
}

void SearchHexDialog::CreateGUIControls()
{
	wxStaticText* inputLabel = new wxStaticText(this, wxID_ANY, "The following hex values will be searched:");
	wxComboBox* input = new wxComboBox(this, wxID_ANY);
	input->Append(s_Offsets);
	input->SetSelection(0);

	wxBoxSizer* inputSizer = new wxBoxSizer(wxVERTICAL);
	inputSizer->Add(inputLabel);
	inputSizer->Add(input, 1, 0, 0);

	wxButton* okButton = new wxButton(this, wxID_ANY, "OK");
	okButton->Bind(wxEVT_BUTTON, &SearchHexDialog::OnOkButton, this);
	wxButton* cancelButton = new wxButton(this, wxID_ANY, "Cancel");

	wxBoxSizer* buttonsSizer = new wxBoxSizer(wxHORIZONTAL);
	buttonsSizer->Add(okButton);
	buttonsSizer->Add(cancelButton);

	wxBoxSizer* rootSizer = new wxBoxSizer(wxVERTICAL);
	rootSizer->Add(inputSizer);
	rootSizer->Add(buttonsSizer);
}

void SearchHexDialog::OnOkButton(wxCommandEvent& event)
{
	EndModal(wxID_OK);
	event.Skip();
}