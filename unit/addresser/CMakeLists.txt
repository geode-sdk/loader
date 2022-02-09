cmake_minimum_required(VERSION 3.0.0)

project(AddresserTest CXX)

add_executable(AddresserTest test.cpp)

target_include_directories(AddresserTest PRIVATE
	${GEODE_INCLUDE_DIR}
	${CMAKE_SOURCE_DIR}/src/utils
) 

project(AddresserTestRun)

add_custom_target(AddresserTestRun ALL
	COMMAND AddresserTest
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	COMMENT "Run AddresserTest"
)
