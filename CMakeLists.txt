CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

include(ExternalProject)
include(${CMAKE_SOURCE_DIR}/cmake/pch.cmake)

PROJECT(kjh-zwave)

find_package(Boost REQUIRED)

set(DIR_OBJECT ${CMAKE_BINARY_DIR}/build)
file(MAKE_DIRECTORY ${DIR_OBJECT})

set(OPENZWAVE_DIR ${CMAKE_SOURCE_DIR}/openzwave/src/openzwave_build/cpp)

ExternalProject_add(openzwave_build
	PREFIX ${CMAKE_SOURCE_DIR}/openzwave
	BINARY_DIR ${CMAKE_SOURCE_DIR}/openzwave/src/openzwave_build
	SVN_REPOSITORY http://open-zwave.googlecode.com/svn/trunk/ 
	BUILD_COMMAND $(MAKE) -C cpp/build/linux
	#BUILD_COMMAND make -C cpp/build/linux -j10
	CONFIGURE_COMMAND echo
	INSTALL_COMMAND echo
	)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++0x") 

# Source Files
file(GLOB CoreSource Core/*.cpp)
file(GLOB CoreHeaders Core/*.h Core/*.hpp)

list(APPEND AllKZWaveFiles ${CoreSource})
list(APPEND AllKZWaveFiles ${CoreHeaders})
list(APPEND AllKZWaveFiles Main.cpp)
list(APPEND AllKZWaveFiles KZWave.h)

include_directories(${CMAKE_SOURCE_DIR})
include_directories(${OPENZWAVE_DIR}/src)
include_directories(${OPENZWAVE_DIR}/src/command_classes)
include_directories(${OPENZWAVE_DIR}/src/value_classes)
include_directories(${OPENZWAVE_DIR}/src/platform)
include_directories(${OPENZWAVE_DIR}/tinyxml)
add_executable(zwaved ${AllKZWaveFiles})

precompiled_header("${AllKZWaveFiles}" pch_KZWave KZWave "")

target_link_libraries(zwaved
	${OPENZWAVE_DIR}/lib/linux/libopenzwave.a
	pthread
	udev
	boost_system
	boost_thread)

add_dependencies(zwaved pch_KZWave)

#add_custom_target(openzwave
#	COMMAND cp ${MOEBIUS_INSTALL_DIR}/lib/* ${DIR_OBJECT}
#	DEPENDS moebius_build)
