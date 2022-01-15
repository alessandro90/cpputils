cmake_minimum_required (VERSION 3.10.0)

add_library(project INTERFACE)
target_include_directories(project INTERFACE .)

install(DIRECTORY "include/" DESTINATION include)
