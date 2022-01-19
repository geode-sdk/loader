cmake_minimum_required(VERSION 3.0 FATAL_ERROR)

project(GeodeLoader VERSION 0.1.0 LANGUAGES C CXX)

file(GLOB CORE_SOURCES src/*.cpp src/internal/*.cpp)

set(SOURCE_FILES main.cpp ${CORE_SOURCES})
include(sdk/SDK/BuildFiles/cmake/CacaoRoot.cmake)

set_target_properties(GeodeLoader PROPERTIES PREFIX "" OUTPUT_NAME "geode")

target_include_directories(GeodeLoader PUBLIC 
	src/internal/
	utils/include/
	core/include/
)
