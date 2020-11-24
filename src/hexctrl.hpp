#pragma once

#include <chrono>

#include <wx/file.h>
#include <wx/dcbuffer.h>
#include <wx/vscroll.h>
#include <wx/filename.h>
#include <wx/timer.h>

#include <moon/table.hpp>
#include <moon/bit_conv.hpp>

class wxHexEvent : public wxNotifyEvent
{	
public:
	wxHexEvent(wxEventType hexType = wxEVT_NULL) :
		wxNotifyEvent(hexType) { }

	wxHexEvent(wxWindowID winId, wxEventType hexType = wxEVT_NULL) :
		wxNotifyEvent(winId, hexType) { }

	wxHexEvent(const wxHexEvent& event) :
		wxNotifyEvent(event) { SetOffset(event.GetOffset()); }

	virtual wxEvent* Clone() const override { return new wxHexEvent(*this);	}	
private:
	uint32_t m_Offset;
public:
	uint32_t GetOffset() const { return m_Offset; }
	void SetOffset(uint32_t offset) { m_Offset = offset; }

	wxDECLARE_DYNAMIC_CLASS(wxHexEvent);
};

typedef void (wxEvtHandler::*wxHexEventFunction)(wxHexEvent&);

wxDECLARE_EVENT(wxEVT_HEX_OFFSET_CHANGED, wxHexEvent);

class wxHexCtrl : public wxHVScrolledWindow
{
public:
	wxHexCtrl(wxWindow* parent, wxWindowID id);
	~wxHexCtrl() = default;
public:	
	void SetTable(const Moon::Hacking::Table& table);

	void SetOffset(size_t offset, bool scroll = false);	
	size_t GetOffset();

	void SetLastOffsetChange();	

	size_t GetDataSize() const { return m_DataSize; }
	const uint8_t* GetData() const { return m_pData; }
	uint8_t* GetData() { return m_pData; }

	void SetData(uint8_t* data);
	void SetData(uint8_t* data, const uint32_t& size);

	size_t GetPageByteCount() { return m_Col*GetRowCount(); }
private:
	void InternalSetOffset(uint32_t offset, bool scroll = false);
private:	
	uint8_t* m_pData = nullptr;
	uint32_t m_DataSize = 0;
	size_t m_Col = 16;
	size_t m_Rows;
	//I don't want to calculate this every time
	size_t m_LastLineSize;
	wxTimer m_UpdateSel;	
//Sub windows stuff
private:	
	wxRect m_ColWindowRect;
	wxRect m_OffsetWindowRect;
	wxRect m_ByteWindowRect;
	wxRect m_CharWindowRect;

	wxPoint GetBytePosition(const size_t& offset, const bool n = 0);
	wxPoint GetCharPosition(const size_t& offset);
//Caret	
private:
	bool m_SelectedByte = true;
	bool m_RightNibble = false;
	bool m_DrawCaret = true;
	size_t m_Offset = 0;
	std::chrono::time_point<std::chrono::high_resolution_clock> m_LastOffsetChange;
	wxColour m_CaretSelBg[2] = { {192, 192, 192}, {0, 0, 255} };	
	wxColour m_CaretSelFore[2] = { {0, 0, 0}, {255, 255, 0} };	
//Selection
private:
	uint32_t m_SelStart = 0;
	uint32_t m_SelLength = 0;
	wxColour m_SelPenColour = { 20, 205, 196 };
	wxColour m_SelBrushColour = { 202, 245, 246 };
	bool IsSquareSelection();
public:
	size_t GetSelStart() const { return m_SelStart; }
	size_t GetSelEnd() const { return m_SelStart+m_SelLength-1; }	
	wxPosition GetSelPosition() const;
	wxPosition GetSelEndPosition() const;
//Table	
private:
	Moon::Hacking::Table m_Table;	
private:
	wxSize m_CharSize;
//Overrides
private:
	virtual wxCoord OnGetRowHeight(size_t row) const;
	virtual wxCoord OnGetColumnWidth(size_t col) const;
//Send Events
	bool SendOffsetChanged(uint32_t newOffset);
//Drawing
private:	
	void CalculateMinSize();
	void OnDraw(wxDC& dc);
	void DrawLines(wxDC& dc);	
	void DrawSeparator(wxDC& dc, wxPoint start, wxPoint end);
	void DrawBytePage(wxDC& dc);
	void DrawCharPage(wxDC& dc);
	void DrawCols(wxDC& dc);
	void DrawOffsets(wxDC& dc);	
	void DrawCaret(wxDC& dc);
	void DrawRect(wxDC& dc, const wxRect& r, const wxColour& c);
	void DrawByte(wxDC& dc, const uint8_t& b, const wxPoint& p, const wxColour& c);
	bool CanDrawChar(const char& _c);
	void DrawSelection(wxDC& dc);
	size_t GetLastDrawingLine();
	//void DrawSelection(wxDC& dc);
//Events
private:
	void OnPaintEvent(wxPaintEvent& event);
	void OnLeftDown(wxMouseEvent& event);
	void OnMouseMove(wxMouseEvent& event);
	void OnSelectionTimer(wxTimerEvent& event);
	void OnResize(wxSizeEvent& event);	
	void OnKeyDown(wxKeyEvent& event);
	void OnChar(wxKeyEvent& event);
};
