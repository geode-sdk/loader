cmake_minimum_required(VERSION 3.3.0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(PROJECT_NAME TestMod2)

project(${PROJECT_NAME} VERSION 1.0.0)

add_library(${PROJECT_NAME} SHARED mod2.cpp)

set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "" LINK_FLAGS "/DELAYLOAD:TestMod1.dll")

target_link_libraries(
    ${PROJECT_NAME}
    GeodeLoader
    TestMod1
)

create_geode_file(${PROJECT_NAME})

