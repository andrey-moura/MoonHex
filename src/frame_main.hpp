#pragma once

#include <wx/frame.h>
#include <wx/menu.h>
#include <wx/filedlg.h>
#include <wx/fswatcher.h>
#include <wx/msgdlg.h>

#include "hexctrl.hpp"
#include "dialog_offset.hpp"
 
class MainFrame : public wxFrame
{
public:
    MainFrame();    
    ~MainFrame() = default;
private:
    wxHexCtrl* m_HexView = nullptr;
    wxFileSystemWatcher m_FileWatcher;

    //In future, the wxHexCtrl will not have access to files...
    wxFile m_File;
    //Why the wxFile have no option to get the path...?
    wxFileName m_FileName;    
    void OnGoOffset();
private:
    void CreateGUIControls();
//Events handlers
    void OnOpenFile();
    void OnOpenTable();
public:
    void OpenFile(const wxString& path);    
//Events
private:
    void OnClose(wxCloseEvent& event);
    void OnMenuClick(wxCommandEvent& event);
    void OnFileWatcher(wxFileSystemWatcherEvent& event);
};

