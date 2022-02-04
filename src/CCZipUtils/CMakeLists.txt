cmake_minimum_required(VERSION 3.0.0)

project(CCZipUtils CXX)

file(GLOB_RECURSE SOURCES *.cpp)

add_library(CCZipUtils ${SOURCES})

if("${GEODE_TARGET_PLATFORM}" STREQUAL "MacOS")
	target_include_directories(CCZipUtils PUBLIC
		"${CMAKE_CURRENT_LIST_DIR}/../../sdk/include/c++stl"
		"${CMAKE_CURRENT_LIST_DIR}/../../sdk/include/cocos/cocos2dx/kazmath/include"
	    "${CMAKE_CURRENT_LIST_DIR}/../../sdk/include/cocos/cocos2dx/support/zip_support"
	    "${CMAKE_CURRENT_LIST_DIR}/../../sdk/include/cocos/cocos2dx"
	    "${CMAKE_CURRENT_LIST_DIR}/../../sdk/include/cocos/cocos2dx/include"
	    "${CMAKE_CURRENT_LIST_DIR}/../../sdk/include/cocos/cocos2dx/platform/mac"
	)
	target_link_libraries(CCZipUtils PUBLIC 
		z
	)
elseif("${GEODE_TARGET_PLATFORM}" STREQUAL "Win32")
	target_include_directories(CCZipUtils PUBLIC
		"${CMAKE_CURRENT_LIST_DIR}/../../sdk/include/"
		"${CMAKE_CURRENT_LIST_DIR}/../../sdk/include/helpers"
		"${CMAKE_CURRENT_LIST_DIR}/../../sdk/include/base"
		"${CMAKE_CURRENT_LIST_DIR}/../../sdk/include/hook"
		"${CMAKE_CURRENT_LIST_DIR}/../../sdk/include/loader"
		"${CMAKE_CURRENT_LIST_DIR}/../../sdk/include/utils"
		"${CMAKE_CURRENT_LIST_DIR}/../../sdk/include/meta"
		"${CMAKE_CURRENT_LIST_DIR}/../../sdk/include/cocos"
		"${CMAKE_CURRENT_LIST_DIR}/../../sdk/include/c++stl"
		"${CMAKE_CURRENT_LIST_DIR}/../../sdk/include/cocos/cocos2dx/kazmath/include"
	    "${CMAKE_CURRENT_LIST_DIR}/../../sdk/include/cocos/cocos2dx/support/zip_support"
	    "${CMAKE_CURRENT_LIST_DIR}/../../sdk/include/cocos/cocos2dx"
	    "${CMAKE_CURRENT_LIST_DIR}/../../sdk/include/cocos/cocos2dx/include"
	    "${CMAKE_CURRENT_LIST_DIR}/../../sdk/include/cocos/cocos2dx/platform/third_party/win32/OGLES"
	    "${CMAKE_CURRENT_LIST_DIR}/../../sdk/include/cocos/cocos2dx/platform/third_party/win32/pthread"
	    "${CMAKE_CURRENT_LIST_DIR}/../../sdk/include/cocos/cocos2dx/platform/third_party/win32/libwebp"
	    "${CMAKE_CURRENT_LIST_DIR}/../../sdk/include/cocos/cocos2dx/platform/third_party/win32/libtiff"
	    "${CMAKE_CURRENT_LIST_DIR}/../../sdk/include/cocos/cocos2dx/platform/third_party/win32/libjpeg"
	    "${CMAKE_CURRENT_LIST_DIR}/../../sdk/include/cocos/cocos2dx/platform/third_party/win32/libpng"
	    "${CMAKE_CURRENT_LIST_DIR}/../../sdk/include/cocos/cocos2dx/platform/third_party/win32/zlib"
	    "${CMAKE_CURRENT_LIST_DIR}/../../sdk/include/cocos/cocos2dx/platform/third_party/win32/iconv"
	    "${CMAKE_CURRENT_LIST_DIR}/../../sdk/include/cocos/cocos2dx/platform/win32"
	)
	target_link_libraries(CCZipUtils PUBLIC 
	    "${CMAKE_CURRENT_LIST_DIR}/zlib.lib"
	)

	add_definitions(-DGEODE_EXPORTING)
elseif("${GEODE_TARGET_PLATFORM}" STREQUAL "iOS")
elseif("${GEODE_TARGET_PLATFORM}" STREQUAL "Android")
else()
endif()


