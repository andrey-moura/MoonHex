#include "dialog_offset.hpp"
#include <moon/bit_conv.hpp>

#include <wx/msgdlg.h>

wxArrayString OffsetDialog::s_LastIndexes;

OffsetDialog::OffsetDialog(size_t currentPos, size_t fileSize) : wxDialog(nullptr, wxID_ANY, "Go To Offset"), m_CurPos(currentPos), m_FileSize(fileSize)
{
	CreateGUIControls();
}

void OffsetDialog::OnButtonClick(wxCommandEvent& event)
{
	if (event.GetId() == wxID_OK)
	{
		std::string s = m_PositionInput->GetValue().ToStdString();

		if(!Moon::BitConverter::IsHexString(s))		
		{
			wxMessageBox(L"Invalid offset. Please type only hex digits (0-9, a-f/A-F)", L"Error", wxICON_ERROR);
			return;
		}

		m_Offset = std::stoi(s, nullptr, 16);
		EndModal(wxID_OK);
	} else 
	{
		EndModal(wxID_CANCEL);
	}
}

void OffsetDialog::CreateGUIControls()
{
	wxStaticText* positionLabel = new wxStaticText(this, wxID_ANY, "New position:");
	m_PositionInput = new wxComboBox(this, wxID_ANY);
	m_PositionInput->Append(s_LastIndexes);

	wxArrayString relativeOptions;
	relativeOptions.Add("beginning");
	relativeOptions.Add("current position");
	relativeOptions.Add("current position (back from)");
	relativeOptions.Add("end (back from)");

	m_RelativeBox = new wxRadioBox(this, wxID_ANY, "relative to...", wxDefaultPosition, wxDefaultSize, relativeOptions, 0, wxRA_VERTICAL);
	m_RelativeBox->SetSelection(0);

	wxBoxSizer* positionSizer = new wxBoxSizer(wxHORIZONTAL);
	positionSizer->Add(positionLabel);
	positionSizer->AddSpacer(4);
	positionSizer->Add(m_PositionInput);

	wxButton* buttonOk = new wxButton(this, wxID_OK, "OK");
	buttonOk->Bind(wxEVT_BUTTON, &OffsetDialog::OnButtonClick, this);
	wxButton* buttonCancel = new wxButton(this, wxID_CANCEL, "Cancel");
	buttonCancel->Bind(wxEVT_BUTTON, &OffsetDialog::OnButtonClick, this);

	wxBoxSizer* buttonsSizer = new wxBoxSizer(wxHORIZONTAL);
	buttonsSizer->Add(buttonOk);
	buttonsSizer->AddStretchSpacer(1);
	buttonsSizer->Add(buttonCancel);

	wxBoxSizer* rootSizer = new wxBoxSizer(wxVERTICAL);
	rootSizer->Add(positionSizer);
	rootSizer->Add(m_RelativeBox);
	rootSizer->Add(buttonsSizer, 1, wxEXPAND, 0);
	SetSizerAndFit(rootSizer);
}