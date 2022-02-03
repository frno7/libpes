cmake_minimum_required(VERSION 3.0)

include_directories(../include)
add_executable(run-tests run-tests.c sax-tests.c svg-transcoder-tests.c)
target_link_libraries(run-tests libpes m)