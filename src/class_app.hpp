#pragma once

#include <wx/app.h>
#include <wx/confbase.h>
#include <wx/fileconf.h>
#include <wx/stdpaths.h>

#include "frame_main.hpp"

class RomHexEditorApp : public wxApp
{
public:
	RomHexEditorApp() = default;
	~RomHexEditorApp() = default;

	virtual bool OnInit();
};

