cmake_minimum_required(VERSION 3.3.0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(PROJECT_NAME TestMod)

project(${PROJECT_NAME} VERSION 1.0.0)

add_library(${PROJECT_NAME} SHARED main.cpp)

add_compile_definitions(EXPORTING_MOD)

set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")

target_link_libraries(
    ${PROJECT_NAME}
    GeodeLoader
)

setup_geode_mod(NO_LOADER BUILD_CODEGEN)
