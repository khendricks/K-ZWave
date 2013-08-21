CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

include(${CMAKE_SOURCE_DIR}/cmake/pch.cmake)

PROJECT(kjh-zwave)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++0x") 

add_subdirectory(libkzwave)
add_subdirectory(kzwave-repl)
