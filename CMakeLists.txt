cmake_minimum_required(VERSION 3.0 FATAL_ERROR)

project(GeodeLoader VERSION 0.1.0 LANGUAGES C CXX)

set(GEODE_NO_GEODE_FILE On)
set(GEODE_BUILD_CODEGEN On)
set(GEODE_NO_LINK_LOADER On)
set(GEODE_NO_IMPLICIT_ENTRY On)

file(GLOB CORE_SOURCES
	src/*.cpp
	src/core/*.cpp
	src/fixes/*.cpp
	src/internal/*.cpp
	src/hooks/*.cpp
	src/gui/loading/*.cpp
	src/gui/mods/*.cpp
	src/gui/mods/dev/*.cpp
	src/gui/mods/info/*.cpp
	src/gui/mods/list/*.cpp
	src/gui/mods/settings/*.cpp
	src/platform/*.cpp
	src/platform/windows/*.cpp
	src/platform/macos/*.cpp
	src/platform/ios/*.cpp
	src/utils/*.cpp
	src/utils/platform/*
)

file(GLOB OBJC_SOURCES
	src/platform/macos/*.mm
	src/platform/ios/*.mm
)

# not recurse because dont include platform specific headers
file(GLOB HEADERS
	sdk/include/*.hpp
	build/gen/Header.hpp
)

set(SOURCE_FILES 
	${CORE_SOURCES}
	${OBJC_SOURCES}
)
set(INCLUDE_DIRECTORIES
	src/internal/
	src/platform/
	src/gui/
	# core/include/
	# core/include/geode/core
)
set(LINK_DIRECTORIES
	# PRIVATE core
)
set(LINK_LIBRARIES
	# geode_core_hook
	CCZipUtils
)

set(BINARY_NAME "Geode")

if (APPLE) 
	if(CMAKE_SYSTEM_PROCESSOR MATCHES "arm" OR GEODE_TARGET_PLATFORM STREQUAL "iOS")
		set (LINK_LIBRARIES ${LINK_LIBRARIES} "-framework UIKit -framework Foundation")
	else()
		set (LINK_LIBRARIES ${LINK_LIBRARIES} "-framework Cocoa")
	endif()
endif()

if (WIN32)
	add_subdirectory(src/inject/windows)
endif()

add_definitions(-DGEODE_EXPORTING)
add_compile_definitions(GEODE_PLATFORM_CONSOLE)

include(sdk/Geode.cmake)

set_source_files_properties(${OBJC_SOURCES} PROPERTIES SKIP_PRECOMPILE_HEADERS ON)

target_precompile_headers(${PROJECT_NAME} PUBLIC ${HEADERS})

# add_subdirectory(core)
add_subdirectory(src/CCZipUtils)

if(NOT GEODE_DONT_BUILD_TEST_MODS)
	if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
		add_subdirectory(test)
	endif()
endif()

if(GEODE_BUILD_UNIT_TESTS)
	if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
		add_subdirectory(unit)
	endif()
endif()
