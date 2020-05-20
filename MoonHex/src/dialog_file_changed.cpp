#include "dialog_file_changed.hpp"

ChangedDialog::ChangedDialog(const wxString& name, bool& dontShowSession) : wxDialog(nullptr, wxID_ANY, "MoonHex"), m_DontShowSeason(dontShowSession)
{
    wxMessageDialog dialog(this,
        "This is a message box\n"
        "This is a long, long string to test out if the message box "
        "is laid out properly.",
        "Message box text",
        wxCENTER |
        wxNO_DEFAULT | wxYES_NO | wxCANCEL |
        wxICON_INFORMATION);    
}