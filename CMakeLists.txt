#Require a minimum version
cmake_minimum_required(VERSION 3.10)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules)
set(CMAKE_CXX_STANDARD 17)

#Set project name
project(MoonHex)

#Don't try to search under Windows
if(WIN32)
	#wxWidgets
	if(DEFINED ENV{WXWIN})		
		message(STATUS "Selecting wxWidgets under environment variable")		
		
		# include and link folder
		include_directories("$ENV{WXWIN}\\include")
		include_directories("$ENV{WXWIN}\\include\\msvc")
		link_directories("$ENV{WXWIN}\\lib\\vc_lib")
		
		# Set characters to Unicode
		SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /UMBCS /D_UNICODE /DUNICODE")
		
		# Group ALL_BUILD and ZERO_CHECK in one folder
		set_property(GLOBAL PROPERTY USE_FOLDERS ON)
		# Set MoonHex as the startup project		
		set_property (DIRECTORY PROPERTY VS_STARTUP_PROJECT "MoonHex")

	endif()
else()
	#wxWidgets
	find_package(wxWidgets REQUIRED COMPONENTS net core base xml)
	include(${wxWidgets_USE_FILE})
endif()

include_directories(include)

SET(SRC
	src/class_app.cpp
	src/class_bit_converter.cpp
	src/dialog_file_changed.cpp
	src/dialog_offset.cpp
	src/dialog_search_hex.cpp
	src/frame_main.cpp
	src/hexctrl.cpp
)

SET(HDR
	src/class_app.hpp
	src/class_bit_converter.hpp
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
