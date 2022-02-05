cmake_minimum_required(VERSION 3.3.0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(PROJECT_NAME TestMod4)

project(${PROJECT_NAME} VERSION 1.0.0)

add_library(${PROJECT_NAME} SHARED mod4.cpp ../../sdk/include/implicitEntry.cpp)

set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")

target_link_libraries(
    ${PROJECT_NAME}
    GeodeLoader
)

create_geode_file(${PROJECT_NAME})
