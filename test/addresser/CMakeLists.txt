cmake_minimum_required(VERSION 3.0.0)

project(AddresserTest CXX)

add_executable(AddresserTest test.cpp)

add_custom_target(AddresserTestRun
	COMMAND AddresserTest
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	COMMENT "Run AddresserTest"
)

add_dependencies(${CMAKE_PROJECT_NAME} AddresserTestRun)
