cmake_minimum_required (VERSION 3.8)

project ("rainy-notification")

add_library(rainy-notification 
	"include/rainy_notification.hpp"
	"src/rainy_notification.cpp"
)

target_include_directories(rainy-notification PRIVATE ${PROJECT_SOURCE_DIR}/include)

target_sources(rainy-notification PRIVATE src/rainy_notification.cpp)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET rainy-notification PROPERTY CXX_STANDARD 20)
endif()