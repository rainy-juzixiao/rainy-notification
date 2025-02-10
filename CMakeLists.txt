cmake_minimum_required (VERSION 3.8)

project ("rainy-notification")

add_executable (rainy-notification  
	"notification.hpp"
	"wintoastlib.cpp"
	"main.cpp"
)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET rainy-notification PROPERTY CXX_STANDARD 20)
endif()

add_compile_options(/utf-8)