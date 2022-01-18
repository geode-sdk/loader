cmake_minimum_required(VERSION 3.0 FATAL_ERROR)

project(LilacLoader VERSION 0.1.0 LANGUAGES C CXX)

#file(GLOB CORE_SOURCES loader-core/*.cpp)

set(CORE_SOURCES loader-core/Hook.cpp)
set(SOURCE_FILES main.cpp ${CORE_SOURCES})
include(Cacao/BuildFiles/cmake/CacaoRoot.cmake)

target_include_directories(LilacLoader PUBLIC loader-core/internal)