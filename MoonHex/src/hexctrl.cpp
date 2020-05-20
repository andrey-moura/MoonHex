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
		m_Selection = Selection();
		m_Selection.selected = true;

		Refresh();
	}

	else
	{
		wxFAIL_MSG_AT("Could not open the file.", "hexctrl.cpp", 42, "wxHexCtrl::OpenFile");
	}
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
	size_t offset = lineStart * m_Col;
	size_t lineEnd = GetVisibleRowsEnd();	

	wxPoint curPos(m_CharSize.GetWidth(), 0);

	size_t charWidth = m_CharSize.GetWidth();
	size_t charHeight = m_CharSize.GetHeight();

	dc.SetBrush(*wxStockGDI::GetBrush(wxStockGDI::Item::BRUSH_BLACK));

	for (size_t line = lineStart; line < lineEnd; ++line)
	{
		std::string offsetText = BitConverter::ToHexString(offset);
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
				char* byteText = BitConverter::ToHexString(m_Data[offset]);
				line.push_back(byteText[0]);
				line.push_back(byteText[1]);
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
	size_t lineSize = m_Col;

	size_t charHeight = m_CharSize.GetHeight();
	wxPoint currentPoint((m_CharsLeftMargin * m_CharSize.GetWidth())+ m_CharSize.GetWidth(), m_CharSize.GetHeight() * posStart.GetRow());

	for (size_t line = posStart.GetRow(); line < posEnd.GetRow(); ++line)
	{
		if ((offset + lineSize) > fileSize)
		{
			while ((offset + lineSize) > fileSize)
			{
				--lineSize;
			}
		}

		std::string lineText((const char*)m_Data + offset, lineSize);
		size_t pos = lineText.find_first_of(m_LineBreak);		

		while (pos != std::string::npos)
		{
			lineText[pos] = ' ';
			pos = lineText.find_first_of(m_LineBreak, pos + 1);
		}
		
		offset += m_Col;

		dc.DrawText(lineText, currentPoint);
		currentPoint.y += charHeight;
	}
}

void wxHexCtrl::DrawSelection(wxDC& dc)
{
	wxPen pen = dc.GetPen();
	wxBrush brush = dc.GetBrush();	

	size_t start = m_Selection.left ? m_LeftMargin : (m_CharsLeftMargin + 1);
	wxRect target(wxPoint((m_Selection.pos.GetCol() + start) * m_CharSize.GetWidth(), m_Selection.pos.GetRow() * m_CharSize.GetHeight()), m_CharSize);

	wxColour penColour;
	wxColour textColour;	

	if (!m_Selection.drawed)
	{
		penColour = wxColour(0, 0, 255);		
		textColour = wxColour(255, 255, 0);		
	}
	else
	{
		penColour = GetBackgroundColour();
		textColour = wxColour(0, 0, 0);
	}

	pen.SetColour(penColour);
	brush.SetColour(penColour);

	dc.SetTextForeground(textColour);
	dc.SetPen(pen);
	dc.SetBrush(brush);

	dc.DrawRectangle(target);
	dc.DrawText(std::string(1, (char)m_Data[m_Selection.offset]), target.GetPosition());

	m_Selection.drawed = !m_Selection.drawed;
}

void wxHexCtrl::OnPaintEvent(wxPaintEvent& event)
{
	wxBufferedPaintDC dc(this);
	PrepareDC(dc);
	OnDraw(dc);
}

void wxHexCtrl::OnLeftDown(wxMouseEvent& event)
{		
	wxPoint pos = event.GetPosition();

	size_t x = pos.x / m_CharSize.GetWidth();

	bool clickedLeft = false;	

	if (x > m_LeftMargin && x < (m_CharsLeftMargin - 1))
	{
		clickedLeft = true;
	}
	else if (x > m_CharsLeftMargin && x <= (m_CharsLeftMargin + m_Col))
	{
		clickedLeft = false;
	}
	else
	{
		event.Skip();
		return;
	}

	wxPosition posStart = GetVisibleBegin();

	size_t row = (pos.y / m_CharSize.GetHeight()) + posStart.GetRow();
	size_t col = x - (clickedLeft ? m_LeftMargin : m_CharsLeftMargin) -1;	

	if (clickedLeft)
		col /= 3;

	size_t offset = (row * m_Col) + col;	

	m_Selection.offset = offset;
	m_Selection.pos.SetCol(col);
	m_Selection.pos.SetRow(row);
	m_Selection.left = clickedLeft;
	m_Selection.selected = true;
	m_Selection.changed = true;
	UpdateSelection();
}

wxCoord wxHexCtrl::OnGetRowHeight(size_t row) const
{
	return wxCoord(m_CharSize.GetHeight());
}

wxCoord wxHexCtrl::OnGetColumnWidth(size_t col) const
{
	return wxCoord(m_CharSize.GetWidth());
}

void wxHexCtrl::UpdateSelection()
{
	if (m_Selection.selected)
	{
		wxClientDC dc(this);
		PrepareDC(dc);

		if (m_Selection.changed)
		{
			m_Selection.drawed = true;
			m_Selection.changed = false;
		}

		DrawSelection(dc);
	}
}

void wxHexCtrl::OnSelectionTimer(wxTimerEvent& event)
{
	UpdateSelection();
	m_UpdateSel.Start(-1);
	event.Skip();
}