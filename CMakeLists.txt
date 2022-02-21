cmake_minimum_required(VERSION 3.0)

project(LibPes VERSION 0.1)

enable_testing()

if(WIN32 AND MSVC)
	set(ADDITIONAL_LIBRARIES)
else()
	set(ADDITIONAL_LIBRARIES m)
endif()

add_subdirectory(src)
add_subdirectory(test)
add_subdirectory(tools)
