cmake_minimum_required(VERSION 3.0 FATAL_ERROR)

project(GeodeLoader VERSION 0.1.0 LANGUAGES C CXX)

set(GEODE_NO_GEODE_FILE On)
set(GEODE_BUILD_CODEGEN On)
set(GEODE_NO_LINK_LOADER On)

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
	set (LINK_LIBRARIES ${LINK_LIBRARIES} "-framework Cocoa")
endif()

if (WIN32)
	add_subdirectory(src/inject/windows)
endif()

add_definitions(-DGEODE_EXPORTING)
add_compile_definitions(GEODE_PLATFORM_CONSOLE)

include(sdk/Geode.cmake)
# target_compile_definitions(GeodeLoader PUBLIC "/w") // what is this
set_target_properties(GeodeLoader PROPERTIES PREFIX "" OUTPUT_NAME "geode")

add_subdirectory(core)
add_subdirectory(src/CCZipUtils)

if(NOT GEODE_DONT_BUILD_TEST_MODS)
	if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
		add_subdirectory(test)
	endif()
endif()
