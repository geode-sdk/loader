cmake_minimum_required(VERSION 3.0.0)

project(AddresserTest CXX)

add_executable(AddresserTest test.cpp)

target_include_directories(AddresserTest PRIVATE
	${GEODE_INCLUDE_DIR}
	${GEODE_INCLUDE_DIR}/cocos/
	${GEODE_INCLUDE_DIR}/cocos/cocos2dx
	${GEODE_INCLUDE_DIR}/cocos/cocos2dx/include
	${GEODE_INCLUDE_DIR}/cocos/cocos2dx/support/zip_support
	${GEODE_INCLUDE_DIR}/cocos/cocos2dx/kazmath/include
	${GEODE_INCLUDE_DIR}/cocos/extensions
	${CMAKE_SOURCE_DIR}/src/utils
) 

project(AddresserTestRun)

add_custom_target(AddresserTestRun ALL
	COMMAND AddresserTest
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	COMMENT "Run AddresserTest"
)
