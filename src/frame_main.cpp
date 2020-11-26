#include "frame_main.hpp"
#include <wx/dir.h>
#include <wx/utils.h>
#include <wx/stdpaths.h>

MainFrame::MainFrame() : wxFrame(nullptr, wxID_ANY, L"MoonHex", wxDefaultPosition, wxDefaultSize, wxMAXIMIZE | wxDEFAULT_FRAME_STYLE)
{	
	CreateGUIControls();

#ifdef __WXGTK__
	wxFont::AddPrivateFont(L"Font/Courier New.ttf");
#endif
}

void MainFrame::CreateGUIControls()
{
	wxMenu* menuFile = new wxMenu();
	menuFile->Append(wxID_OPEN, "Open");
	menuFile->Append(13000, "Open Table");
	menuFile->Append(wxID_CLOSE, "Close");

	wxMenu* menuNavigation = new wxMenu();
	menuNavigation->Append(wxID_INDEX, "Go To Offset...\tCTRL-G");

	wxMenuBar* menuBar = new wxMenuBar();
	menuBar->Append(menuFile, "File");
	menuBar->Append(menuNavigation, "Navigation");
	menuBar->Bind(wxEVT_MENU, &MainFrame::OnMenuClick, this);
	SetMenuBar(menuBar);

	m_HexView = new wxHexCtrl(this, wxID_ANY);
	m_HexView->Bind(wxEVT_HEX_OFFSET_CHANGED, &MainFrame::OnOffsetChanged, this);

	wxBoxSizer* rootSizer = new wxBoxSizer(wxVERTICAL);
	rootSizer->Add(m_HexView, 1, wxEXPAND, 0);	

	SetStatusBar(CreateStatusBar(2, wxSTB_SHOW_TIPS | wxSTB_ELLIPSIZE_END | wxFULL_REPAINT_ON_RESIZE));	

	int widths[] = { -1 , -1 };
	int styles[] = { wxSB_RAISED, wxSB_RAISED };

	m_frameStatusBar->SetStatusWidths(2, widths);
	m_frameStatusBar->SetStatusStyles(2, styles);	

	SetSizer(rootSizer);	
}

void MainFrame::OnOpenFile()
{
	wxString path = m_FileName.GetPath();	

	wxFileDialog dialog(nullptr, "Select a binary file", path);

	if (dialog.ShowModal() != wxID_CANCEL)
		OpenFile(dialog.GetPath());
}

void MainFrame::OnOpenTable()
{	
	wxFileDialog dialog(nullptr, "Select a table file");

	if (dialog.ShowModal() != wxID_CANCEL)
		m_HexView->SetTable(Moon::Hacking::Table(dialog.GetPath().ToStdString()));
}

void MainFrame::OpenFile(const wxString& path)
{
	if (!m_File.Open(path))
		return;

	wxString old_path = m_FileName.GetFullPath();

	m_FileName = path;

	SetTitle(m_FileName.GetFullName());

	wxFile file(path);	

	if(m_HexView->GetDataSize() == file.Length() && m_HexView->GetData() && old_path == m_FileName.GetFullPath())
	{
		uint8_t* data = m_HexView->GetData();
		file.Read(data, file.Length());

		m_HexView->SetData(data);
	}
	else 
	{
		uint8_t* data = new uint8_t[file.Length()];		
		file.Read(data, file.Length());

		delete[] m_HexView->GetData();

		m_HexView->SetData(data, file.Length());
	}	
}

void MainFrame::OnGoOffset()
{
	OffsetDialog dialog(m_HexView->GetOffset(), m_HexView->GetDataSize());
	if (dialog.ShowModal() != wxID_CANCEL)
	{
		m_HexView->SetOffset(dialog.GetOffset(), true);
	}
}

void MainFrame::OnMenuClick(wxCommandEvent& event)
{
	int id = event.GetId();	

	switch (id)
	{
	case wxID_OPEN:
		OnOpenFile();
		break;
	case 13000:
		OnOpenTable();
		break;
	case wxID_CLOSE:
		break;
	case wxID_INDEX:
		OnGoOffset();
		break;
	default:

		break;
	}

	event.Skip();
}

void MainFrame::OnOffsetChanged(wxHexEvent& event)
{
	uint32_t offset = event.GetOffset();
	const uint8_t* data = m_HexView->GetData();

	SetStatusText(wxString::Format(L"Offset: %s", Moon::BitConverter::ToHexString(offset)), 0);
	SetStatusText(wxString::Format(L"Value: %s", std::to_wstring(data[offset])), 1);

	event.Skip();
}

void MainFrame::OnFileWatcher(wxFileSystemWatcherEvent& event)
{
	wxString path = event.GetPath().GetFullPath();
	wxString myPath = m_FileName.GetFullPath().RemoveLast();

	if (path == myPath)
	{
		if (event.GetChangeType() == wxFSW_EVENT_MODIFY)
		{
			wxMessageDialog dialog(nullptr,
				wxString("\"") << path <<
				"\" was changed outside of this session." <<
				"Lose current state and reload?", "MoonHex",
				wxCENTER | wxNO_DEFAULT | wxYES_NO | wxCANCEL | wxICON_QUESTION);

			if (dialog.ShowModal() != wxCANCEL)
			{				
				OpenFile(m_FileName.GetFullPath());				
			}
		}
	}	

	event.Skip();
}

void MainFrame::OnClose(wxCloseEvent& event)
{
	event.Skip();
}