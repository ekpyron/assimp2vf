cmake_minimum_required (VERSION 3.0)
project (assimp2vf)

include_directories (contrib)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

add_subdirectory (src)
