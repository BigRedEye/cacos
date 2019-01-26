cmake_minimum_required(VERSION 3.5)

project(dtl)

add_library(${PROJECT_NAME} INTERFACE)
add_library(dtl::dtl ALIAS ${PROJECT_NAME})

target_include_directories(
    ${PROJECT_NAME} INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/dtl/
)
