project( akinator )
cmake_minimum_required(VERSION 2.8)
# Debug, Release, RelWithDebInfo and MinSizeRe
set(CMAKE_BUILD_TYPE RelWithDebInfo)
SET(CMAKE_VERBOSE_MAKEFILE OFF)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra") # add extra warnings

add_executable( test_AkinatorDialog
  test_AkinatorDialog.cpp
  AkinatorDialog.cpp
  AkinatorDialog.h
  CAkinator.cpp
  CAkinator.h
)
