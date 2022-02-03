cmake_minimum_required(VERSION 3.0)

file(GLOB LIBRARY_SOURCES *.c *.h)

include_directories(../include)
add_library(libpes ${LIBRARY_SOURCES})
