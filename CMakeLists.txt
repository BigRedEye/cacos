cmake_minimum_required(VERSION 3.5)

project(cacos)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    SET( CMAKE_CXX_FLAGS_SANITIZERS "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=undefined,address -fsanitize-undefined-trap-on-error"
        CACHE STRING "Flags for C++ with UBSan and ASan enabled."
        FORCE)
    SET( CMAKE_C_FLAGS_SANITIZERS "${CMAKE_C_FLAGS_DEBUG} -fsanitize=undefined,address -fsanitize-undefined-trap-on-error"
        CACHE STRING "Flags for C with UBSan and ASan enabled."
        FORCE)
    SET( CMAKE_EXE_LINKER_FLAGS_SANITIZERS
        "${CMAKE_EXE_LINKER_FLAGS_DEBUG}" CACHE STRING
        "Flags for linker with UBSan and ASan enabled."
        FORCE )
    SET( CMAKE_SHARED_LINKER_FLAGS_SANITIZERS
        "${CMAKE_SHARED_LINKER_FLAGS_DEBUG}" CACHE STRING
        "Flags for linker with UBSan and ASan enabled."
        FORCE )
    MARK_AS_ADVANCED(
        CMAKE_CXX_FLAGS_SANITIZERS
        CMAKE_C_FLAGS_SANITIZERS
        CMAKE_EXE_LINKER_FLAGS_SANITIZERS
        CMAKE_SHARED_LINKER_FLAGS_SANITIZERS )
    # Update the documentation string of CMAKE_BUILD_TYPE for GUIs
    SET( CMAKE_BUILD_TYPE "${CMAKE_BUILD_TYPE}" CACHE STRING
        "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel Sanitizers."
        FORCE )
endif()

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
