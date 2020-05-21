#pragma once

#include <wx/file.h>
#include <wx/dcbuffer.h>
#include <wx/vscroll.h>
#include <wx/filename.h>
#include <wx/timer.h>

#include <moon/table.hpp>

#include "class_bit_converter.hpp"

#ifdef _DEBUG
#define SEPARATOR_COLOR 197,0,0
#else 
#define SEPARATOR_COLOR 200,200,200
#endif

struct Selection
{
	wxPosition pos;
	size_t offset;
	bool selected;
	bool drawed;
	bool left;
	bool changed;

	Selection() : pos(0, 0), offset(0), selected(false), drawed(false), left(false), changed(false)
	{

	}
};

class wxHexCtrl : public wxHVScrolledWindow
{
public:
	wxHexCtrl(wxWindow* parent, wxWindowID id);
	~wxHexCtrl() = default;
public:
	void OpenFile(const wxString& path);
	void OpenTable(const wxString& path);
	void SetOffset(size_t offset);
	void UpdateSelection();
	size_t GetOffset();
	size_t GetFileSize() { return m_File.Length(); }
private:
	wxFile m_File;
	uint8_t* m_Data = nullptr;
	size_t m_Col = 16;	
	Selection m_Selection;	
	wxTimer m_UpdateSel;	

	Moon::Hacking::Table m_Table;	
	void TestTable();
private:
	wxSize m_CharSize;
	wxRect m_LineSize;
private:	
	size_t m_LeftMargin;
	size_t m_CharsLeftMargin;
//Overrides
private:
	virtual wxCoord OnGetRowHeight(size_t row) const;
	virtual wxCoord OnGetColumnWidth(size_t col) const;
//Drawing
private:	
	void CalculateMinSize();
	void OnDraw(wxDC& dc);
	void DrawLines(wxDC& dc);
	void DrawSeparator(wxDC& dc);
	void DrawBytePage(wxDC& dc);
	void DrawCharPage(wxDC& dc);
	void DrawOffsets(wxDC& dc);	
	void DrawSelection(wxDC& dc);
//Events
private:
	void OnPaintEvent(wxPaintEvent& event);
	void OnLeftDown(wxMouseEvent& event);		
	void OnSelectionTimer(wxTimerEvent& event);
};