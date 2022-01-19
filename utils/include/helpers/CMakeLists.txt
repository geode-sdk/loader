cmake_minimum_required(VERSION 3.0.0)

project(geode_utils CXX)

file(GLOB_RECURSE UTILS_SOURCES 
	src/*.cpp
)

add_library(geode_utils ${UTILS_SOURCES})

target_include_directories(
    geode_utils PUBLIC
	"${CMAKE_CURRENT_SOURCE_DIR}"
	"${CMAKE_CURRENT_SOURCE_DIR}/.."
)

target_link_libraries(
	geode_utils PUBLIC
	geode_core_hook
)
