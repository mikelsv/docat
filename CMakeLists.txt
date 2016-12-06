cmake_minimum_required(VERSION 3.1)

project(docat)

set(SOURCE docat.cpp ../../opensource/msvcore2/VString.cpp ../../opensource/msvcore2/MString.cpp)

set (CMAKE_CXX_STANDARD 11)

add_executable(allsort ${SOURCE})


