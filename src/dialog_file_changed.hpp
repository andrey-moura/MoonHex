#include <wx/wx.h>

class ChangedDialog : public wxDialog
{
public:
	ChangedDialog(const wxString& name, bool& dontShowSession);
private:
	bool m_DontShowSeason;
};
