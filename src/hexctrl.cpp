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
	Bind(wxEVT_KEY_DOWN, &wxHexCtrl::OnKeyDown, this);
	Bind(wxEVT_CHAR, &wxHexCtrl::OnChar, this);
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

	size_t lineStart = GetVisibleRowsBegin();
	size_t lineEnd = GetVisibleRowsEnd();
	size_t lineCount = lineEnd - lineStart;


	if((lineCount * OnGetRowHeight(0)) > GetSize().GetHeight())
	{
		//Okay, so if the last line is not completely shown, we do not draw it.
		lineCount--;
	}

	m_Rows = lineCount;
}

void wxHexCtrl::SetOffset(size_t offset, bool scroll)
{
	SetLastOffsetChange();	

	m_Offset = offset;

	if(scroll)
		ScrollToRow(offset / m_Col);

	Refresh();
}

void wxHexCtrl::SetLastOffsetChange()
{
	m_DrawCaret = true;
	m_LastOffsetChange = std::chrono::high_resolution_clock::now();
}

size_t wxHexCtrl::GetOffset()
{
	return m_Offset;
}

wxPoint wxHexCtrl::GetBytePosition(const size_t& offset, const bool n)
{
	wxPoint point = m_ByteWindowRect.GetPosition();
	point.x += m_CharSize.GetWidth(); //padding
	point.x += m_CharSize.GetWidth() * n; // + 0 to left nibble and + m_CharSize.GetWidth() to right nibble
	point.x += (offset % m_Col) * (m_CharSize.GetWidth()*3);
	point.y += (offset / m_Col) * m_CharSize.GetHeight();

	return point;
}

wxPoint wxHexCtrl::GetCharPosition(const size_t& offset)
{
	wxPoint point = m_CharWindowRect.GetPosition();
	point.x += m_CharSize.GetWidth(); //padding
	point.x += (offset % m_Col) * m_CharSize.GetWidth();
	point.y += (offset / m_Col) * m_CharSize.GetHeight();

	return point;
}

void wxHexCtrl::DrawOffsets(wxDC& dc)
{	
	dc.SetTextForeground(wxColour(0, 0, 191, 255));

	size_t lineStart = GetVisibleRowsBegin();
	uint32_t offset = lineStart * m_Col;
	size_t lineEnd = GetLastDrawingLine();

	wxPoint curPos(m_CharSize.GetWidth(), 0);

	size_t charWidth = m_CharSize.GetWidth();
	size_t charHeight = m_CharSize.GetHeight();	

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
	DrawCaret(dc);
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

		size_t lineEnd = GetLastDrawingLine();

		for (size_t y = posStart.GetRow(); y < lineEnd; ++y)
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

	size_t lastLine = GetLastDrawingLine();

	for (size_t line = posStart.GetRow(); line < lastLine; ++line)
	{
		size_t lineSize = m_Col;

		if ((offset + lineSize) > fileSize)
		{
			lineSize = fileSize-offset;
		}

		lineText.clear();
		
		for(size_t i = 0; i < lineSize; ++i)
		{
			uint8_t c = *(m_Data + offset + i);
			//m_Table.Input(c);

			if(CanDrawChar(c))
			{
				lineText.push_back((char)c);				
			}
			else 
			{
				lineText.push_back(' ');
			}
		}		
		
		offset += m_Col;
				
		wxString s(lineText.c_str(), wxCSConv(wxFONTENCODING_CP1252), lineText.size());

		dc.DrawText(s, currentPoint);
		currentPoint.y += charHeight;
	}

	DrawSeparator(dc, m_CharWindowRect.GetRightTop(), m_CharWindowRect.GetRightBottom());
}

void wxHexCtrl::DrawRect(wxDC& dc, const wxRect& r, const wxColour& c)
{
	dc.SetPen(*wxTRANSPARENT_PEN);
	dc.SetBrush(wxBrush(c, wxBRUSHSTYLE_SOLID));
	dc.DrawRectangle(r);
}

void wxHexCtrl::DrawByte(wxDC& dc, const uint8_t& b, const wxPoint& r, const wxColour& c)
{
	std::string hex_str = Moon::BitConverter::ToHexChar(m_Data[m_Offset]);	

	dc.SetTextForeground(c);
	dc.DrawText(hex_str[m_RightNibble], r);
}

void wxHexCtrl::DrawCaret(wxDC& dc)
{
	if((m_Offset / m_Col) == GetLastDrawingLine())
	{
		return;
	}		

	if(m_SelectedByte && m_DrawCaret || !m_SelectedByte)
	{
		DrawRect(dc, wxRect(GetBytePosition(m_Offset, m_RightNibble), m_CharSize), m_CaretSelBg[m_SelectedByte]);
		DrawByte(dc, m_Data[m_Offset], GetBytePosition(m_Offset, m_RightNibble), m_CaretSelFore[m_SelectedByte]);
	}	

	if(!m_SelectedByte && m_DrawCaret || m_SelectedByte)
	{
		DrawRect(dc, wxRect(GetCharPosition(m_Offset), m_CharSize), m_CaretSelBg[!m_SelectedByte]);	

		char c = *((const char*)m_Data + m_Offset);
		m_Table.Input(c);

		if(!CanDrawChar(c))
		{
			c = ' ';
		}

		wxString s(&c, wxCSConv(wxFONTENCODING_CP1252), 1);
		dc.SetTextForeground(m_CaretSelFore[!m_SelectedByte]);
		dc.DrawText(s, GetCharPosition(m_Offset));
	}
}

bool wxHexCtrl::CanDrawChar(const char& _c)
{
	uint8_t c = (uint8_t)_c;

	if(c >= 0x20  && c != 0x7F)// &&
			    //c != 0x81 && c != 0x6F &&
				//c != 0x90 && c != 0x9C &&
				//c != 0x9F && c != 0x7F)
	{
		return true;
	}

	return false;
}

size_t wxHexCtrl::GetLastDrawingLine()
{
	return m_Rows + GetVisibleRowsBegin();	
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
	wxPoint position = event.GetPosition();	

	if(m_ByteWindowRect.Contains(position))
	{		
		position.x -= m_ByteWindowRect.x;
		position.x += GetVisibleColumnsBegin();

		size_t x = position.x / (m_CharSize.x*3);
		size_t n = (position.x / m_CharSize.x) - (x*3);

		if(n == 0)
			return; //padding			

		m_RightNibble = n-1;
		size_t y = (position.y / m_CharSize.y);
		y += GetVisibleRowsBegin();

		SetOffset(x+(y*m_Col));

		m_SelectedByte = true;

		event.Skip(false);
		Refresh();
	} else if(m_CharWindowRect.Contains(position))
	{
		position.x -= m_CharWindowRect.x;		
		position.x += GetVisibleColumnsBegin();

		size_t x = position.x / m_CharSize.x;
			
		if(x == 0)
			return; //padding

		if(x > m_Col)
			return;

		x--;			
		
		size_t y = (position.y / m_CharSize.y);

		if(y > m_Rows)
			return;

		y += GetVisibleRowsBegin();

		SetOffset(x+(y*m_Col));

		m_SelectedByte = false;

		event.Skip(false);
		Refresh();
	} else 
	{
		event.Skip();
	}
}

void wxHexCtrl::OnResize(wxSizeEvent& event)
{
	CalculateMinSize();
	event.Skip();
}

void wxHexCtrl::OnKeyDown(wxKeyEvent& event)
{
	switch(event.GetKeyCode())
	{
		case wxKeyCode::WXK_LEFT:			

			if((!m_RightNibble || !m_SelectedByte))
			{
				if(m_Offset == 0)
				{
					return;
				}

				SetOffset(m_Offset-1);
			}

			if(m_SelectedByte)
			{
				m_RightNibble = !m_RightNibble;
			}
			
			SetLastOffsetChange();
		break;
		case wxKeyCode::WXK_RIGHT:

			if(m_RightNibble || (!m_SelectedByte))
			{
				SetOffset(m_Offset+1);
			}
			
			if(m_SelectedByte)
			{
				m_RightNibble = !m_RightNibble;
			}

			SetLastOffsetChange();
		break;
		case wxKeyCode::WXK_UP:

			if(m_Offset >= m_Col)
			{
				if((m_Offset / m_Col) - GetVisibleRowsBegin() == 0)
				{
					ScrollToRow(GetVisibleRowsBegin()-1);
				}

				SetOffset(m_Offset-m_Col);
			}

		break;
		case wxKeyCode::WXK_DOWN:			
			SetOffset(m_Offset+m_Col);

			if((m_Offset / m_Col) > GetLastDrawingLine()-1)
			{
				ScrollToRow(GetVisibleRowsBegin()+1);
			}
		break;
		case wxKeyCode::WXK_TAB:
			m_SelectedByte = !m_SelectedByte;

			if(!m_SelectedByte)
			{
				m_RightNibble = false;
			}
		break;
		case wxKeyCode::WXK_PAGEUP:
			if(m_Offset >= m_Rows*m_Col)
			{
				SetOffset(m_Offset - m_Rows*m_Col, true);
			}			
		break;
		case wxKeyCode::WXK_PAGEDOWN:						
			SetOffset(m_Offset + m_Rows*m_Col, true);
		break;		
		default:
			event.Skip();
		break;
	}

	Refresh();	
}

void wxHexCtrl::OnChar(wxKeyEvent& event)
{	
	event.Skip();

	char c = (char)event.GetUnicodeKey();
	if(!CanDrawChar(c))
	{		
		return;
	}	

	if(m_SelectedByte)
	{
		wxString str = event.GetUnicodeKey();
		str = str.Upper();
		uint8_t c = str[0];

		if(!isxdigit(c))
		{			
			return;
		}

		m_Data[m_Offset] = Moon::BitConverter::ReplaceNibble(
		m_Data[m_Offset],
		Moon::BitConverter::FromHexNibble(c),
		!m_RightNibble);

		if(m_RightNibble)
		{
			++m_Offset;
		} 
		
		m_RightNibble = !m_RightNibble;		
		event.Skip(false);
	} else 
	{	
		uint8_t c = event.GetUnicodeKey();
		m_Data[m_Offset] = c;
		++m_Offset;
		event.Skip(false);
	}	
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
	auto now = std::chrono::high_resolution_clock::now();

	if(std::chrono::duration_cast<std::chrono::milliseconds>(now - m_LastOffsetChange).count() >= m_UpdateSel.GetInterval())
	{
		m_DrawCaret = !m_DrawCaret;
		m_UpdateSel.Start(-1);
		Refresh();
	} else
	{
		m_DrawCaret = true;
	}

	event.Skip();
}