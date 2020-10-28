#include "hexctrl.hpp"

wxHexCtrl::wxHexCtrl(wxWindow* parent, wxWindowID id) : wxHVScrolledWindow(parent, id)
{
	SetFont(wxFontInfo(10).FaceName("Courier New"));
	SetBackgroundColour(wxColor(255, 255, 255, 255));
	CalculateMinSize();
	SetBackgroundStyle(wxBG_STYLE_PAINT);
	m_UpdateSel.Start(600);	

	m_UpdateSel.Bind(wxEVT_TIMER, &wxHexCtrl::OnSelectionTimer, this);
	Bind(wxEVT_PAINT, &wxHexCtrl::OnPaintEvent, this);
	Bind(wxEVT_LEFT_DOWN, &wxHexCtrl::OnLeftDown, this);
	Bind(wxEVT_SIZE, &wxHexCtrl::OnResize, this);
	Bind(wxEVT_MOTION, &wxHexCtrl::OnMouseMove, this);
}

void wxHexCtrl::OpenFile(const wxString& path)
{
	if (m_File.IsOpened())
		m_File.Close();

	if (m_Data != nullptr)
		delete[] m_Data;

	if (m_File.Open(path))
	{
		size_t size = m_File.Length();

		m_Data = new uint8_t[size];

		m_File.Read(m_Data, size);

		CalculateMinSize();

		ScrollToRow(0);		

		Refresh();
	}

	m_Caret.drawed = false;
	m_Caret.left = true;
	m_Caret.offset = 0;
	m_Caret.rect.x = 0;
	m_Caret.rect.y = 0;
}

void wxHexCtrl::OpenTable(const wxString& path)
{
	m_Table.Open(path.ToStdString());	
	Refresh();
}

void wxHexCtrl::CalculateMinSize()
{		
	if (!m_File.IsOpened())
		return;

	size_t rows = m_File.Length() / m_Col;

	if (rows % m_Col != 0)
		++rows;

	m_CharSize = GetTextExtent("A");	

	m_OffsetWindowRect.width = m_CharSize.GetWidth()*10 /* 8 character + one margin left and right*/;
	m_OffsetWindowRect.height = GetSize().GetHeight();
	m_OffsetWindowRect.SetPosition({ 0, 0 });

	m_ByteWindowRect.width = ((m_CharSize.GetWidth()*3)*m_Col) + m_CharSize.GetWidth()/* one char for each nibble + one space for each byte */; //+ one padding on right
	m_ByteWindowRect.height = GetSize().GetHeight();
	m_ByteWindowRect.SetPosition(m_OffsetWindowRect.GetRightTop());

	m_CharWindowRect.width = (m_CharSize.GetWidth()*(m_Col+2 /* + right and left margin */));
	m_CharWindowRect.height = GetSize().GetHeight();
	m_CharWindowRect.SetPosition(m_ByteWindowRect.GetRightTop());

	SetRowColumnCount(rows, m_OffsetWindowRect.width+m_ByteWindowRect.width+m_CharWindowRect.width);
}

 void wxHexCtrl::SetOffset(size_t offset)
 {
	 ScrollToRow(offset / m_Col);	 
	 Refresh();
 }

 size_t wxHexCtrl::GetOffset()
 {
	 return m_Col * GetVisibleRowsBegin();
 }

void wxHexCtrl::DrawOffsets(wxDC& dc)
{	
	dc.SetTextForeground(wxColour(0, 0, 191, 255));

	size_t lineStart = GetVisibleRowsBegin();
	uint32_t offset = lineStart * m_Col;
	size_t lineEnd = GetVisibleRowsEnd();	

	wxPoint curPos(m_CharSize.GetWidth(), 0);

	size_t charWidth = m_CharSize.GetWidth();
	size_t charHeight = m_CharSize.GetHeight();

	dc.SetBrush(*wxStockGDI::GetBrush(wxStockGDI::Item::BRUSH_BLACK));

	for (size_t line = lineStart; line < lineEnd; ++line)
	{
		std::string offsetText = Moon::BitConverter::ToHexString(offset);
		offset += m_Col;

		dc.DrawText(offsetText, charWidth, charHeight * line);
	}

	DrawSeparator(dc, m_OffsetWindowRect.GetRightTop(), m_OffsetWindowRect.GetRightBottom());
}

void wxHexCtrl::OnDraw(wxDC& dc)
{	
	dc.Clear();	

	if (!m_File.IsOpened())
		return;

	DrawOffsets(dc);
	DrawLines(dc);	
	DrawBytePage(dc);
	DrawCharPage(dc);
}

void wxHexCtrl::DrawLines(wxDC& dc)
{
#ifdef _DEBUG
	wxPen pen = dc.GetPen();
	pen.SetStyle(wxPENSTYLE_SOLID);
	pen.SetWidth(1);
	pen.SetColour(200, 200, 200);
	dc.SetPen(pen);
		
	dc.SetBrush(*wxTRANSPARENT_BRUSH);

	wxPosition posStart = GetVisibleBegin();
	wxPosition posEnd = GetVisibleEnd();

	for (size_t y = posStart.GetRow(); y < posEnd.GetRow(); ++y)
	{
		for (size_t x = posStart.GetCol(); x < posEnd.GetCol(); ++x)
		{			
			dc.DrawRectangle(wxPoint(x * m_CharSize.GetWidth(), y * m_CharSize.GetHeight()), m_CharSize);		
		}		
	}
#endif // _DEBUG
}

void wxHexCtrl::DrawSeparator(wxDC& dc, wxPoint start, wxPoint end)
{	
	dc.SetPen(wxPen(wxColour(SEPARATOR_COLOR), 1, wxPENSTYLE_SOLID));

	wxPosition posStart = GetVisibleBegin();

	start.x += posStart.GetCol()*m_CharSize.GetWidth();
	start.y += posStart.GetRow()*m_CharSize.GetHeight();

	end.x += posStart.GetCol()*m_CharSize.GetWidth();
	end.y += posStart.GetRow()*m_CharSize.GetHeight();

	dc.DrawLine(start, end);
}

void wxHexCtrl::DrawBytePage(wxDC& dc)
{
	if(m_Data != nullptr)
	{
		dc.SetTextForeground(wxColour(0, 0, 0));

		size_t fileSize = m_File.Length();		

		wxPosition posStart = GetVisibleBegin();
		wxPosition posEnd = GetVisibleEnd();

		size_t offset = m_Col * posStart.GetRow();

		std::string line;
		line.reserve((m_Col * 3));

		size_t charHeight = m_CharSize.GetHeight();
		wxPoint currentPoint = wxPoint(m_ByteWindowRect.x+m_CharSize.GetWidth(), posStart.GetRow() *charHeight);

		for (size_t y = posStart.GetRow(); y < posEnd.GetRow(); ++y)
		{			
			for (size_t col = 0; col < m_Col; ++col)
			{
				std::string byteText = Moon::BitConverter::ToHexString(m_Data[offset]);
				line.append(byteText);
				line.push_back(' ');

				offset++;

				if (offset == fileSize)
					break;
			}

			dc.DrawText(line, currentPoint);
			currentPoint.y += charHeight;

			if (offset == fileSize)
				break;

			line.clear();
		}
	}

	DrawSeparator(dc, m_ByteWindowRect.GetTopRight(), m_ByteWindowRect.GetBottomRight());
}

void wxHexCtrl::DrawCharPage(wxDC& dc)
{
	wxPosition posStart = GetVisibleBegin();
	wxPosition posEnd = GetVisibleEnd();

	size_t fileSize = m_File.Length();
	size_t offset = posStart.GetRow() * m_Col;	

	size_t charHeight = m_CharSize.GetHeight();
	wxPoint currentPoint(m_CharWindowRect.x+m_CharSize.GetWidth(), m_CharSize.GetHeight() * posStart.GetRow());

	std::string lineText;
	lineText.reserve(m_Col);

	for (size_t line = posStart.GetRow(); line < posEnd.GetRow(); ++line)
	{
		size_t lineSize = m_Col;

		if ((offset + lineSize) > fileSize)
		{
			lineSize = fileSize-offset;
		}

		lineText.clear();
		
		for(size_t i = 0; i < lineSize; ++i)
		{
			char c  = *((const char*)m_Data + offset + i);

			if(	isprint(c) )
				lineText.push_back(c);
		}
		
		m_Table.Input(lineText);
		
		offset += m_Col;

		dc.DrawText(lineText, currentPoint);		
		currentPoint.y += charHeight;
	}

	DrawSeparator(dc, m_CharWindowRect.GetRightTop(), m_CharWindowRect.GetRightBottom());
}

void wxHexCtrl::OnPaintEvent(wxPaintEvent& event)
{
	wxBufferedPaintDC dc(this);
	PrepareDC(dc);
	OnDraw(dc);
}

void wxHexCtrl::OnMouseMove(wxMouseEvent& event)
{	
	if (event.GetButton() == wxMOUSE_BTN_NONE)
	{
		if (m_OffsetWindowRect.Contains(event.GetPosition()))
		{		
			SetCursor(wxCURSOR_RIGHT_ARROW);
		}	
		else
		{
			SetCursor(wxCURSOR_ARROW);
		}
	}

	event.Skip();
}

void wxHexCtrl::OnLeftDown(wxMouseEvent& event)
{		
	wxPoint point = event.GetPosition();

	if(m_OffsetWindowRect.Contains(point))
	{
		SetBackgroundColour(*wxRED);		
	} else if(m_ByteWindowRect.Contains(point))
	{
		SetBackgroundColour(*wxBLUE);
	} else if(m_CharWindowRect.Contains(point))
	{
		SetBackgroundColour(*wxGREEN);
	} else 
	{
		SetBackgroundColour(*wxWHITE);
	}

	event.Skip();
}

void wxHexCtrl::OnResize(wxSizeEvent& event)
{
	ScrollToColumn(0);
	ScrollToRow(0);

	event.Skip();
}

wxCoord wxHexCtrl::OnGetRowHeight(size_t row) const
{
	return wxCoord(m_CharSize.GetHeight());
}

wxCoord wxHexCtrl::OnGetColumnWidth(size_t col) const
{
	return wxCoord(m_CharSize.GetWidth());
}

void wxHexCtrl::OnSelectionTimer(wxTimerEvent& event)
{
	//UpdateSelection();
	m_UpdateSel.Start(-1);
	event.Skip();
}
