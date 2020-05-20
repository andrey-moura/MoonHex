#include "dialog_offset.hpp"

wxArrayString OffsetDialog::s_LastIndexes;
std::string OffsetDialog::s_Numbers = "0123456789";
std::string OffsetDialog::s_Hex = "0123456789ABCDEF";

OffsetDialog::OffsetDialog(size_t currentPos, size_t fileSize) : wxDialog(nullptr, wxID_ANY, "Go To Offset"), m_CurPos(currentPos), m_FileSize(fileSize)
{
	CreateGUIControls();
}

void OffsetDialog::OnButtonClick(wxCommandEvent& event)
{
	int id = event.GetId();

	if (id == wxID_OK)
	{
		std::string s = m_PositionInput->GetValue().ToStdString();
		size_t offset = 0;

		if (s[0] == '0' && s[1] == 'x')
		{
			if (s.find_first_not_of(s_Hex, 2) == std::string::npos)
			{
				std::string toConvert = s.substr(2);
				offset = std::stoi(toConvert, nullptr, 16);
			}
		}
		else if (s.find_first_not_of(s_Numbers, 2) != std::string::npos)
		{
			std::string toConvert = s.substr(2);
			offset = std::stoi(toConvert);
		}
		else
		{
			id = wxID_CANCEL;
		}

		size_t index = m_RelativeBox->GetSelection();

		//if (index == 1)
		//{
		//	offset += m_CurPos;
		//}
		//else if (index == 2)
		//{
		//	offset = m_CurPos -= offset;
		//}
		//else if (index == 3)
		//{
		//	offset = m_FileSize - offset;
		//}

		m_Offset = offset;
	}
	
	EndModal(id);
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