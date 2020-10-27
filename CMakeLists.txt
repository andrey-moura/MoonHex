#Require a minimum version
cmake_minimum_required(VERSION 3.10)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules)
set (CMAKE_CXX_STANDARD 17)

#Set project name
project(MoonHex)

#wxWidgets
find_package(wxWidgets REQUIRED COMPONENTS net core base xml)
include(${wxWidgets_USE_FILE})

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

#Add the executable
add_executable(MoonHex ${SRC} ${HDR})

target_link_libraries(MoonHex ${wxWidgets_LIBRARIES})
