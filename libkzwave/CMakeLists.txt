CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

include(ExternalProject)

find_package(Boost REQUIRED)
find_package(SWIG REQUIRED)
find_package(PythonLibs REQUIRED)

include(${SWIG_USE_FILE})

set(OPENZWAVE_DIR ${CMAKE_BINARY_DIR}/openzwave/src/openzwave/cpp PARENT_SCOPE)
set(OPENZWAVE_DIR ${CMAKE_BINARY_DIR}/openzwave/src/openzwave/cpp)

set(LIBKZWAVE_DIR ${CMAKE_CURRENT_SOURCE_DIR} PARENT_SCOPE)

ExternalProject_add(openzwave
	PREFIX ${CMAKE_BINARY_DIR}/openzwave
	BINARY_DIR ${CMAKE_BINARY_DIR}/openzwave/src/openzwave_build
	SVN_REPOSITORY http://open-zwave.googlecode.com/svn/trunk/ 
	BUILD_COMMAND $(MAKE) -C ${OPENZWAVE_DIR}/build/linux
	CONFIGURE_COMMAND echo
	INSTALL_COMMAND echo
	)

# Source Files
file(GLOB CoreSource Core/*.cpp)
file(GLOB CoreHeaders Core/*.h Core/*.hpp)

list(APPEND Alllibkzwavefiles ${CoreSource})
list(APPEND Alllibkzwavefiles ${CoreHeaders})
list(APPEND Alllibkzwavefiles kzwave.h)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${OPENZWAVE_DIR}/src)
include_directories(${OPENZWAVE_DIR}/src/command_classes)
include_directories(${OPENZWAVE_DIR}/src/value_classes)
include_directories(${OPENZWAVE_DIR}/src/platform)
include_directories(${OPENZWAVE_DIR}/tinyxml)
include_directories(${PYTHON_INCLUDE_PATH})
add_library(kzwave SHARED ${Alllibkzwavefiles})

precompiled_header("${Alllibkzwavefiles}" pch_KZWave kzwave "")

target_link_libraries(kzwave
	${OPENZWAVE_DIR}/lib/linux/libopenzwave.a
	pthread
	udev
	boost_system
	boost_thread
	python2.7
	)


add_dependencies(pch_KZWave openzwave)
add_dependencies(kzwave pch_KZWave openzwave)

install(TARGETS kzwave
	DESTINATION lib)

SET_SOURCE_FILES_PROPERTIES(kzwave.i PROPERTIES CPLUSPLUS ON)
SET_SOURCE_FILES_PROPERTIES(kzwave.i PROPERTIES SWIG_FLAGS "-includeall")
SWIG_ADD_MODULE(kzwave python kzwave.i )
SWIG_LINK_LIBRARIES(kzwave
	pthread
	udev
	boost_system
	boost_thread
	python2.7
	kzwave
	${PYTHON_LIBRARIES})

#SWIG_LINK_LIBRARIES(kzwave-swig
#	${OPENZWAVE_DIR}/lib/linux/libopenzwave.a
#	pthread
#	udev
#	boost_system
#	boost_thread
#	python2.7
#	)

#add_dependencies(kzwave-swig pch_KZWave openzwave)
