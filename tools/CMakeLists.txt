cmake_minimum_required(VERSION 3.0)

include_directories(../include)

add_library(fileutils file.c file.h)

add_executable(pes-info pes-info.c)
target_link_libraries(pes-info libpes fileutils)

add_executable(pes-to-svg-emb pes-to-svg-emb.c)
target_link_libraries(pes-to-svg-emb libpes fileutils)

add_executable(svg-emb-to-pes svg-emb-to-pes.c)
target_link_libraries(svg-emb-to-pes libpes fileutils m)