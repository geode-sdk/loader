cmake_minimum_required(VERSION 3.0 FATAL_ERROR)

project(GeodeLoader VERSION 0.1.0 LANGUAGES C CXX)

set(GEODE_NO_GEODE_FILE On)
set(GEODE_BUILD_CODEGEN On)
set(GEODE_NO_LINK_LOADER On)
set(GEODE_IMPLICIT_ENTRY Off)

file(GLOB CORE_SOURCES
	src/*.cpp
	src/fixes/*.cpp
	src/helper/*.cpp
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
	src/platform/macos/*.mm
	src/platform/macos/*.cpp
	src/platform/ios/*.mm
	src/platform/ios/*.cpp
	src/utils/*.cpp
	src/utils/platform/*
)

set(SOURCE_FILES 
	${CORE_SOURCES}
)
set(INCLUDE_DIRECTORIES
	src/internal/
	src/platform/
	src/gui/
	core/include/
	core/include/geode/core
)
set(LINK_DIRECTORIES
	PRIVATE core
)
set(LINK_LIBRARIES
	geode_core_hook
	CCZipUtils
)

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
# target_compile_definitions(GeodeLoader PUBLIC "/w") // what is this
set_target_properties(GeodeLoader PROPERTIES PREFIX "" OUTPUT_NAME "Geode")

add_subdirectory(core)
add_subdirectory(src/CCZipUtils)
# add_subdirectory(src/addressertest)

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
