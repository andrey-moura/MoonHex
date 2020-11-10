#include "hexctrl.hpp"

wxHexCtrl::wxHexCtrl(wxWindow* parent, wxWindowID id) : wxHVScrolledWindow(parent, id)
{	
	wxFontInfo info = wxFontInfo(10).FaceName("Courier New");
	wxFontStyle style = info.GetStyle();

	SetFont(info);
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

	size_t rows = 0;

	if(m_File.Length() < m_Col)
	{
		rows = 1;
	}
	else 
	{
		rows = m_File.Length() / m_Col;

		if (rows % m_Col != 0)
			++rows;
	}		

	m_CharSize = GetTextExtent("A");		

	m_ColWindowRect.SetPosition({0, 0});	
	m_ColWindowRect.height = m_CharSize.GetHeight();

	m_OffsetWindowRect.width = m_CharSize.GetWidth()*10 /* 8 character + one margin left and right*/;
	m_OffsetWindowRect.height = GetSize().GetHeight();
	m_OffsetWindowRect.SetPosition(m_ColWindowRect.GetLeftBottom());

	m_ByteWindowRect.width = ((m_CharSize.GetWidth()*3)*m_Col) + m_CharSize.GetWidth()/* one char for each nibble + one space for each byte */; //+ one padding on right
	m_ByteWindowRect.height = GetSize().GetHeight();
	m_ByteWindowRect.SetPosition(m_OffsetWindowRect.GetRightTop());

	m_CharWindowRect.width = (m_CharSize.GetWidth()*(m_Col+2 /* + right and left margin */));
	m_CharWindowRect.height = GetSize().GetHeight();
	m_CharWindowRect.SetPosition(m_ByteWindowRect.GetRightTop());

	size_t width = m_OffsetWindowRect.width+m_ByteWindowRect.width+m_CharWindowRect.width;

	SetRowColumnCount(rows+1, width/m_CharSize.GetWidth());	
	m_ColWindowRect.SetBottomRight(m_CharWindowRect.GetTopRight());

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

//-----------------------------------------------------------------------------------------------------------------//

wxPosition wxHexCtrl::GetSelPosition() const
{
	return wxPosition(m_SelStart / m_Col, m_SelStart % m_Col);
}

wxPosition wxHexCtrl::GetSelEndPosition() const
{
	return wxPosition(GetSelEnd() / m_Col, GetSelEnd() % m_Col);
}

bool wxHexCtrl::IsSquareSelection()
{	
	wxPosition start_sel = GetSelPosition();
	wxPosition end_sel = GetSelEndPosition();

	if(start_sel.GetRow() == end_sel.GetRow())
		return true;

	if(start_sel.GetCol() == 0 && end_sel.GetCol() == m_Col-1)
		return true;

	return false;
}

//-----------------------------------------------------------------------------------------------------------------//

void wxHexCtrl::DrawCols(wxDC& dc)
{
	dc.SetTextForeground(wxColour(0, 0, 191, 255));
	wxPoint cur_pos = m_ColWindowRect.GetPosition();
	cur_pos.y = GetVisibleRowsBegin()*m_CharSize.GetHeight();

	cur_pos.x += m_CharSize.GetWidth()*2; //padding

	wxString label = L"Offset";

	dc.DrawText(label, cur_pos);

	cur_pos.x += label.size()*m_CharSize.GetWidth();
	cur_pos.x += m_CharSize.GetWidth()*2; //padding
	cur_pos.x -= 1;	

	for(uint8_t i = 0; i < m_Col; ++i)
	{
		wxPoint p = GetBytePosition(i);
		cur_pos.x = p.x;

		std::string hex_str = Moon::BitConverter::ToHexString<uint8_t>(i);

		dc.DrawText(hex_str, cur_pos);
	}

	wxString ancii = L"ANSI ASCII";

	size_t ancii_width = ancii.size() * m_CharSize.GetWidth();
	size_t char_page_width = m_CharWindowRect.width;

	size_t x = (char_page_width - ancii_width) / 2;
	x += m_CharWindowRect.x;
	cur_pos.x = x;

	dc.SetTextForeground(wxColour(130, 122, 190));
	dc.DrawText(ancii, cur_pos);	

	DrawSeparator(dc, m_ColWindowRect.GetBottomLeft(), m_ColWindowRect.GetBottomRight());
}

void wxHexCtrl::DrawOffsets(wxDC& dc)
{	
	dc.SetTextForeground(wxColour(0, 0, 191, 255));

	size_t lineStart = GetVisibleRowsBegin();
	uint32_t offset = lineStart * m_Col;
	size_t lineEnd = GetLastDrawingLine();

	wxPoint curPos(m_OffsetWindowRect.GetPosition());
	curPos.x += m_CharSize.GetWidth(); //padding

	curPos.y += lineStart*m_CharSize.GetHeight();

	size_t charWidth = m_CharSize.GetWidth();
	size_t charHeight = m_CharSize.GetHeight();	

	for (size_t line = lineStart; line < lineEnd; ++line)
	{
		std::string offsetText = Moon::BitConverter::ToHexString(offset);
		offset += m_Col;

		dc.DrawText(offsetText, curPos);
		curPos.y += charHeight;
	}

	wxPoint p = m_OffsetWindowRect.GetRightTop();
	p.y = 0;

	DrawSeparator(dc, p, m_OffsetWindowRect.GetRightBottom());
}

void wxHexCtrl::OnDraw(wxDC& dc)
{	
	dc.Clear();	

	if (!m_File.IsOpened())
		return;

	DrawSelection(dc);
	DrawCols(dc);
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
		wxPoint currentPoint = m_ByteWindowRect.GetPosition();		
		currentPoint.x += m_CharSize.GetWidth(); //padding
		currentPoint.y += posStart.GetRow()*m_CharSize.GetHeight();

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

	wxPoint p = m_ByteWindowRect.GetTopRight();
	p.y = 0;
	DrawSeparator(dc, p, m_ByteWindowRect.GetBottomRight());
}

void wxHexCtrl::DrawCharPage(wxDC& dc)
{
	wxPosition posStart = GetVisibleBegin();
	wxPosition posEnd = GetVisibleEnd();

	size_t fileSize = m_File.Length();
	size_t offset = posStart.GetRow() * m_Col;	

	size_t charHeight = m_CharSize.GetHeight();
	wxPoint currentPoint(m_CharWindowRect.x+m_CharSize.GetWidth(), m_CharSize.GetHeight() * posStart.GetRow());
	currentPoint.y += m_CharSize.GetHeight();

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

	wxPoint p = m_CharWindowRect.GetRightTop();
	p.y = 0;
	DrawSeparator(dc, p, m_CharWindowRect.GetRightBottom());
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

void wxHexCtrl::DrawSelection(wxDC& dc)
{
	if(m_SelLength == 0)
		return;

	wxPosition sel_start = GetSelPosition();
	wxPosition sel_end = GetSelEndPosition();

	if(sel_start.GetRow() < GetVisibleRowsBegin())
		return;

	if(sel_end.GetRow() > GetVisibleRowsEnd())
		return;

	wxPen pen(m_SelPenColour, 2, wxPENSTYLE_SOLID);
	wxBrush brush(m_SelBrushColour, wxBRUSHSTYLE_SOLID);		

	dc.SetPen(*wxTRANSPARENT_PEN);

	if(IsSquareSelection())
	{
		//dc.SetPen(pen);
		dc.SetBrush(brush);

		wxPoint left_top = GetCharPosition(m_SelStart);
		wxPoint right_bottom = GetCharPosition(GetSelEnd());

		right_bottom.y += m_CharSize.GetHeight();
		right_bottom.x += m_CharSize.GetWidth();

		wxRect r(left_top, right_bottom);

		dc.DrawRectangle(r);
	} else 
	{
		wxPoint p = GetCharPosition(m_SelStart);		
		size_t first_width = (m_Col-sel_start.GetCol())*m_CharSize.GetWidth();
		
		dc.SetBrush(brush);
		dc.DrawRectangle(p.x, p.y, first_width, m_CharSize.GetHeight());

		// dc.SetPen(*wxTRANSPARENT_PEN);

		for(size_t line = sel_start.GetRow()+1; line < sel_end.GetRow(); ++line)
		{
		 	p.y += m_CharSize.GetHeight();
		 	dc.DrawRectangle(p.x, p.y, m_Col * m_CharSize.GetWidth(), m_CharSize.GetHeight());
		}

		// size_t line_end_offset = end_line * m_Col;
		// size_t cols2 = end_offset - line_end_offset;

		p.y += m_CharSize.GetHeight();
		dc.DrawRectangle(p.x, p.y, sel_end.GetCol() * m_CharSize.GetWidth(), m_CharSize.GetHeight());

		// dc.SetPen(pen);
		
		// dc.DrawLine(p.x, GetCharPosition(line_start+1).y*m_CharSize.GetHeight(), p.x, GetCharPosition(end_line-1).y*m_CharSize.GetHeight());
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

		if(y == 0)
			return;				

		y += GetVisibleRowsBegin()-1;

		if(y > m_Rows)
			return;

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

		if(y == 0)
			return;				

		y += GetVisibleRowsBegin()-1;

		if(y > m_Rows)
			return;

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

			if(event.ShiftDown())
			{
				if(m_SelLength == 0)
				{
					m_SelStart = m_Offset;
					m_SelLength = 1;
				} else
				{
					if(m_SelectedByte)
					{
						if(m_RightNibble)
						{
							m_SelLength++;
						}
					}
					else 
					{
						m_SelLength++;
					}					
				}
			}

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