#pragma once

#include <wx/file.h>
#include <wx/dcbuffer.h>
#include <wx/vscroll.h>
#include <wx/filename.h>
#include <wx/timer.h>

#include <moon/table.hpp>
#include <moon/bit_conv.hpp>

#ifdef _DEBUG
#define SEPARATOR_COLOR 197,0,0
#else 
#define SEPARATOR_COLOR 200,200,200
#endif

//Because of wxCaret...
struct HexCaret
{
	wxRect rect;	
	size_t offset;	
	wxColour background;
	wxColour foreground;
	bool drawed;
	bool left;

	HexCaret() : background(0, 0, 128)
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
	//void UpdateSelection();
	size_t GetOffset();
	size_t GetFileSize() { return m_File.Length(); }
	size_t GetPageByteCount() { return m_Col*GetRowCount(); }
private:
	wxFile m_File;
	uint8_t* m_Data = nullptr;
	size_t m_Col = 16;		
	wxTimer m_UpdateSel;	
//Sub windows stuff
private:	
	wxRect m_OffsetWindowRect;	
	wxRect m_ByteWindowRect;
	wxRect m_CharWindowRect;
//Caret	
private:
	HexCaret m_HexCaret;
	void UpdateCaretPosition();
	void ResetCaret()
	{
		m_HexCaret.drawed = false;
		m_HexCaret.left = true;
		m_HexCaret.offset = 0;
		UpdateCaretPosition();
	}
//Table	
private:
	Moon::Hacking::Table m_Table;	
private:
	wxSize m_CharSize;
//Overrides
private:
	virtual wxCoord OnGetRowHeight(size_t row) const;
	virtual wxCoord OnGetColumnWidth(size_t col) const;
//Drawing
private:	
	void CalculateMinSize();
	void OnDraw(wxDC& dc);
	void DrawLines(wxDC& dc);	
	void DrawSeparator(wxDC& dc, wxPoint start, wxPoint end);
	void DrawBytePage(wxDC& dc);
	void DrawCharPage(wxDC& dc);
	void DrawOffsets(wxDC& dc);	
	void DrawCursor(wxDC& dc);
	//void DrawSelection(wxDC& dc);
//Events
private:
	void OnPaintEvent(wxPaintEvent& event);
	void OnLeftDown(wxMouseEvent& event);
	void OnMouseMove(wxMouseEvent& event);
	void OnSelectionTimer(wxTimerEvent& event);
	void OnResize(wxSizeEvent& event);
	void OnKeyDown(wxKeyEvent& event);
};
