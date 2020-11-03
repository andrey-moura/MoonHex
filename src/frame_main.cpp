#include "frame_main.hpp"
#include <wx/dir.h>
#include <wx/utils.h>
#include <wx/stdpaths.h>

MainFrame::MainFrame() : wxFrame(nullptr, wxID_ANY, "MoonHex")
{	
	CreateGUIControls();

	Bind(wxEVT_CLOSE_WINDOW, &MainFrame::OnClose, this);
	m_FileWatcher.Bind(wxEVT_FSWATCHER, &MainFrame::OnFileWatcher, this);
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

	wxBoxSizer* rootSizer = new wxBoxSizer(wxVERTICAL);
	rootSizer->Add(m_HexView, 1, wxEXPAND, 0);	

	SetSizer(rootSizer);	
}

void MainFrame::OnOpenFile()
{
	wxFileDialog dialog(nullptr, "Select a binary file");

	if (dialog.ShowModal() != wxID_CANCEL)
		OpenFile(dialog.GetPath());
}

void MainFrame::OnOpenTable()
{
	wxFileDialog dialog(nullptr, "Select a table file");

	if (dialog.ShowModal() != wxID_CANCEL)
		m_HexView->OpenTable(dialog.GetPath());
}

void MainFrame::OpenFile(const wxString& path)
{
	if (!m_File.Open(path))
		return;

	m_FileName.SetPath(path);	

	SetTitle(m_FileName.GetFullName());
	m_HexView->OpenFile(path);

	m_FileWatcher.RemoveAll();

	wxFileName fn = wxFileName::DirName(path.substr(0, path.find_last_of(m_FileName.GetPathSeparator())));

	m_FileWatcher.Add(fn);
}

void MainFrame::OnGoOffset()
{
	OffsetDialog dialog(m_HexView->GetOffset(), m_HexView->GetFileSize());
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
				m_HexView->OpenFile(m_FileName.GetFullPath());
			}
		}
	}	

	event.Skip();
}

void MainFrame::OnClose(wxCloseEvent& event)
{
	m_FileWatcher.Unbind(wxEVT_FSWATCHER, &MainFrame::OnFileWatcher, this);

	event.Skip();
}