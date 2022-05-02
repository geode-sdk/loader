cmake_minimum_required(VERSION 3.18 FATAL_ERROR)

project(GeodeLoader VERSION 0.1.0 LANGUAGES C CXX)

set(GEODE_NO_GEODE_FILE On)
set(GEODE_BUILD_CODEGEN On)
set(GEODE_NO_LINK_LOADER On)
set(GEODE_NO_IMPLICIT_ENTRY On)

set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(READ about.md LOADER_ABOUT_MD)
configure_file(src/internal/about.hpp.in ${CMAKE_SOURCE_DIR}/src/internal/about.hpp)

file(GLOB CORE_SOURCES
	src/main.cpp
	src/cocos2d-ext/*.cpp
	src/core/*.cpp
	src/hooks/*.cpp
	src/internal/*.cpp
	src/internal/windows/*.cpp
	src/load/*.cpp
	src/load/windows/*.cpp
	src/utils/*.cpp
	src/utils/zip/*.cpp
	src/mac/*.cpp
	src/utils/windows.cpp
)

file(GLOB OBJC_SOURCES
	src/internal/mac/*.mm
	src/internal/ios/*.mm
	src/load/mac/*.mm
	src/load/ios/*.mm
	src/utils/mac/*.mm
	src/utils/ios/*.mm
)

# not recurse because dont include platform specific headers
file(GLOB HEADERS
	sdk/include/*.hpp
)

add_library(${PROJECT_NAME} SHARED
	${CORE_SOURCES}
	${OBJC_SOURCES}
)

target_include_directories(${PROJECT_NAME} PUBLIC
	src/internal/
	src/platform/
	src/gui/
	dobby/include/
)
target_link_directories(${PROJECT_NAME} PRIVATE dobby ${CMAKE_CURRENT_SOURCE_DIR}/link)
target_link_libraries(${PROJECT_NAME} dobby z)

if (WIN32)
	add_subdirectory(launcher/windows)
endif()

add_definitions(-DGEODE_EXPORTING)
add_compile_definitions(GEODE_PLATFORM_CONSOLE)
find_path(GEODE_SDK_PATH
  NAMES Geode.cmake
  PATHS $ENV{GEODE_SUITE} /usr/local/geode/sdk ~/.geode/sdk
  DOC "Geode SDK path."
  REQUIRED
)

include(${GEODE_SDK_PATH}/Geode.cmake)
setup_geode_mod(BUILD_CODEGEN OUTPUT "Geode")

if (APPLE) 
	if(CMAKE_SYSTEM_PROCESSOR MATCHES "arm" OR GEODE_TARGET_PLATFORM STREQUAL "iOS")
		add_custom_command(TARGET GeodeLoader
		    POST_BUILD COMMAND 
		    ${CMAKE_INSTALL_NAME_TOOL} -id \"/Library/MobileSubstrate/DynamicLibraries/Geode.dylib\"
		    $<TARGET_FILE:GeodeLoader>)
	endif()
endif()

set_source_files_properties(${OBJC_SOURCES} PROPERTIES SKIP_PRECOMPILE_HEADERS ON)

target_precompile_headers(${PROJECT_NAME} PUBLIC ${HEADERS})

SET(DOBBY_GENERATE_SHARED OFF CACHE BOOL "Build dobby shared library" FORCE)
# SET(DOBBY_DEBUG ON CACHE BOOL "Build dobby shared library" FORCE)
add_subdirectory(dobby)

if(NOT GEODE_DONT_BUILD_TEST_MODS)
	if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
		add_subdirectory(test)
	endif()
endif()
