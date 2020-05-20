#pragma once

#include <wx/app.h>

#include "frame_main.hpp"

class RomHexEditorApp : public wxApp
{
public:
	RomHexEditorApp() = default;
	~RomHexEditorApp() = default;

	virtual bool OnInit();
};

