#Require a minimum version
cmake_minimum_required(VERSION 3.10)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules)
set(CMAKE_CXX_STANDARD 17)

#Set project name
project(MoonHex)

#Don't try to search under Windows
if(WIN32)
	if(DEFINED ENV{SDK})		
		message(STATUS "Found SDK folder: $ENV{SDK}")
		
		#wxWidgets
		if(EXISTS "$ENV{SDK}\\wxWidgets")
			set(wxWidgets_PATH "$ENV{SDK}\\wxWidgets")
			message(STATUS "Found wxWidgets: ${wxWidgets_PATH}")
			
			# wxWidgets include directory
			include_directories("${wxWidgets_PATH}\\include")
			include_directories("${wxWidgets_PATH}\\include\\msvc")
			# wxWidgets library folder
			if(${CMAKE_CL_64})
				link_directories("${wxWidgets_PATH}\\lib\\vc_x64_lib")
			else()
				link_directories("${wxWidgets_PATH}\\lib\\vc_lib")
			endif()			
		endif()
	endif()
	# Set characters to Unicode
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /UMBCS /D_UNICODE /DUNICODE")
			
	# Group ALL_BUILD and ZERO_CHECK in one folder
	set_property(GLOBAL PROPERTY USE_FOLDERS ON)
	# Set Uva Engine as the startup project
	set_property (DIRECTORY PROPERTY VS_STARTUP_PROJECT "UvaEngine")
	# Disable these annoying warnings
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)

	set_property(GLOBAL APPEND PROPERTY LINK_FLAGS /MACHINE:X86)

else()
	#wxWidgets
	find_package(wxWidgets REQUIRED COMPONENTS net core base xml)
	include(${wxWidgets_USE_FILE})
endif()

include_directories(include)

SET(SRC
	src/class_app.cpp
	src/dialog_file_changed.cpp
	src/dialog_offset.cpp
	src/dialog_search_hex.cpp
	src/frame_main.cpp
	src/hexctrl.cpp
	src/dialog_preferences.cpp
)

SET(HDR
	src/class_app.hpp
	src/dialog_file_changed.hpp
	src/dialog_offset.hpp
	src/dialog_search_hex.hpp
	src/frame_main.hpp
	src/hexctrl.hpp
)

if(WIN32)
	add_executable(MoonHex WIN32 ${SRC} ${HDR})	
else()
	add_executable(MoonHex ${SRC} ${HDR})	
	target_link_libraries(MoonHex ${wxWidgets_LIBRARIES})
endif()
