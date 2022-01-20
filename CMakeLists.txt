cmake_minimum_required(VERSION 3.0 FATAL_ERROR)

project(GeodeLoader VERSION 0.1.0 LANGUAGES C CXX)

set(GEODE_NO_GEODE_FILE On)

file(GLOB CORE_SOURCES
	src/*.cpp
	src/internal/*.cpp
	src/gui/*.cpp
	src/platform/*.cpp
	src/utils/*.cpp
	src/utils/windows*.cpp
)

add_subdirectory(core)
add_subdirectory(src/CCZipUtils)

set(SOURCE_FILES
	${CORE_SOURCES}
)
set(INCLUDE_DIRECTORIES
	src/internal/
	src/gui/
	core/include/
	core/include/geode/core
)
set(LINK_DIRECTORIES
	PRIVATE core
)
set(LINK_LIBRARIES
	geode_core_hook
)

add_definitions(-DGEODE_EXPORTING)

include(sdk/SDK/BuildFiles/cmake/Root.cmake)
target_compile_definitions(GeodeLoader PUBLIC "/w")
set_target_properties(GeodeLoader PROPERTIES PREFIX "" OUTPUT_NAME "geode")

target_link_libraries(GeodeLoader CCZipUtils)

