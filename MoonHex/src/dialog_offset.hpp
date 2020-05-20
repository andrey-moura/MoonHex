#pragma once

#include <wx/dialog.h>
#include <wx/stattext.h>
#include <wx/choice.h>
#include <wx/radiobox.h>
#include <wx/combobox.h>
#include <wx/button.h>
#include <wx/sizer.h>

class OffsetDialog : public wxDialog
{
public:
	OffsetDialog(size_t currentPos, size_t fileSize);
private:
	static wxArrayString s_LastIndexes;
	static std::string s_Numbers;
	static std::string s_Hex;

public:
	size_t GetOffset() { return m_Offset; }
private:
	void CreateGUIControls();
	void OnButtonClick(wxCommandEvent& event);
	wxRadioBox* m_RelativeBox = nullptr;
	wxComboBox* m_PositionInput = nullptr;
	size_t m_Offset = 0;
	size_t m_CurPos = 0;
	size_t m_FileSize = 0;
};