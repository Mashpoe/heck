cmake_minimum_required(VERSION 3.10)

# project name
project(Heck)

file(GLOB_RECURSE SourceFiles src/*.c)
add_executable(heck ${SourceFiles})

include_directories(include)