cmake_minimum_required(VERSION 3.9)

project(cacos_test)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(DownloadGoogleTest)
include(Sanitizers)

file(GLOB SOURCES
    inline_variables.cpp
    main.cpp
    split.cpp
)

file(GLOB HEADERS
)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

target_link_libraries(${PROJECT_NAME} PUBLIC cacos_lib gtest_main gtest)

add_test(UnitTest ${PROJECT_NAME})
