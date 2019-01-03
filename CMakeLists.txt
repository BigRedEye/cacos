cmake_minimum_required(VERSION 3.5)

project(cacos)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(Sanitizers)

file(GLOB SOURCES
    src/main.cpp
)

file(GLOB HEADERS
)

add_executable(${PROJECT_NAME})

target_sources(${PROJECT_NAME} PUBLIC
    ${HEADERS} ${SOURCES}
)

find_package(Threads)

SET(Boost_USE_MULTITHREAD ON)
find_package(Boost REQUIRED COMPONENTS system thread)
target_include_directories(${PROJECT_NAME} PUBLIC ${Boost_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PUBLIC ${Boost_LINK_LIBS})

add_subdirectory(third-party/cpparg)
add_subdirectory(third-party/termcolor)

set(JSON_BuildTests OFF CACHE INTERNAL "")
add_subdirectory(third-party/json)

target_link_libraries(${PROJECT_NAME}
    cpparg
    termcolor::termcolor
    nlohmann_json::nlohmann_json
    Boost::system
    ${CMAKE_THREAD_LIBS_INIT}
)
