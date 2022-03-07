cmake_minimum_required(VERSION 3.3.0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(PROJECT_NAME TestMod8)

project(${PROJECT_NAME} VERSION 1.0.0)

add_library(${PROJECT_NAME} SHARED mod8.cpp)

set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")

target_compile_definitions(${PROJECT_NAME} PUBLIC -DEXPORT_TestMod8=1)

target_link_libraries(
    ${PROJECT_NAME}
    GeodeLoader
)

create_geode_file(${PROJECT_NAME})
