CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

find_package(Boost REQUIRED)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${OPENZWAVE_DIR}/src)
include_directories(${OPENZWAVE_DIR}/src/command_classes)
include_directories(${OPENZWAVE_DIR}/src/value_classes)
include_directories(${OPENZWAVE_DIR}/src/platform)
include_directories(${OPENZWAVE_DIR}/tinyxml)
include_directories(${LIBKZWAVE_DIR})
add_executable(kzwave-interactive KZWave/Main.cpp)

target_link_libraries(kzwave-interactive
	kzwave
	pthread
	udev
	boost_system
	boost_thread
	python2.7
	)

add_custom_target(config_dir
	COMMAND cmake -E copy_directory ${OPENZWAVE_DIR}/../config ${CMAKE_CURRENT_BINARY_DIR}/config
	DEPENDS openzwave)

add_dependencies(kzwave-interactive kzwave config_dir)

install(TARGETS kzwave-interactive
	DESTINATION bin)
