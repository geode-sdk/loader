cmake_minimum_required(VERSION 3.0 FATAL_ERROR)

project(GeodeLoader VERSION 0.1.0 LANGUAGES C CXX)

file(GLOB CORE_SOURCES src/*.cpp src/internal/*.cpp)

add_subdirectory(core)

set(SOURCE_FILES 
	main.cpp 
	${CORE_SOURCES}
)
set(INCLUDE_DIRECTORIES
	src/internal/
	utils/include/
	core/include/
	core/include/geode/core
)
set(LINK_DIRECTORIES
	PRIVATE core
)
set(LINK_LIBRARIES
	geode_core_hook
)
include(sdk/SDK/BuildFiles/cmake/Root.cmake)

set_target_properties(GeodeLoader PROPERTIES PREFIX "" OUTPUT_NAME "geode")

