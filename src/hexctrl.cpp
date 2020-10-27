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
	
	m_LeftMargin = 10;
	m_CharsLeftMargin = m_LeftMargin + (m_Col * 3) + 1;

	m_CharSize = GetTextExtent("A");
	SetRowColumnCount(rows, m_CharsLeftMargin + m_Col + 1);
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
}

void wxHexCtrl::OnDraw(wxDC& dc)
{	
	dc.Clear();	

	if (!m_File.IsOpened())
		return;

	DrawOffsets(dc);
	DrawLines(dc);
	DrawSeparator(dc);
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

void wxHexCtrl::DrawSeparator(wxDC& dc)
{
	wxPen pen = dc.GetPen();
	pen.SetColour(wxColour(SEPARATOR_COLOR));
	dc.SetPen(pen);

	size_t leftMargin = m_LeftMargin * m_CharSize.GetWidth();
	size_t height = m_CharSize.GetHeight() * GetRowCount();
	size_t charsMargin = m_CharsLeftMargin * m_CharSize.GetWidth();

	dc.DrawLine(wxPoint(leftMargin, 0), wxPoint(leftMargin, height));
	dc.DrawLine(wxPoint(charsMargin, 0), wxPoint(charsMargin, height));
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
		wxPoint currentPoint = wxPoint((m_LeftMargin * m_CharSize.GetWidth()) + m_CharSize.GetWidth(), posStart.GetRow() *charHeight);

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
}

void wxHexCtrl::DrawCharPage(wxDC& dc)
{
	wxPosition posStart = GetVisibleBegin();
	wxPosition posEnd = GetVisibleEnd();

	size_t fileSize = m_File.Length();
	size_t offset = posStart.GetRow() * m_Col;	

	size_t charHeight = m_CharSize.GetHeight();
	wxPoint currentPoint((m_CharsLeftMargin * m_CharSize.GetWidth())+ m_CharSize.GetWidth(), m_CharSize.GetHeight() * posStart.GetRow());

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
}

void wxHexCtrl::OnPaintEvent(wxPaintEvent& event)
{
	wxBufferedPaintDC dc(this);
	PrepareDC(dc);
	OnDraw(dc);
}

void wxHexCtrl::OnMouseMove(wxMouseEvent& event)
{
	wxPoint point = event.GetPosition();	

	if (event.GetButton() == wxMOUSE_BTN_NONE)
	{
		if (point.x < (m_LeftMargin * m_CharSize.GetWidth()))
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

	if (point.x > (m_CharsLeftMargin + 1) * m_CharSize.GetWidth())
	{		
		if (point.x < ((m_Col * 3) * m_CharSize.GetWidth()))
		{
			std::string();
		}
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
