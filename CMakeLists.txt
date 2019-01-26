cmake_minimum_required(VERSION 3.12)

project(cacos)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(CACOS_BUILD_TESTS "Build tests" OFF)
option(CACOS_DOWNLOAD_BOOST "Do not use system boost" OFF)
option(BUILD_STATIC_LIBS "Prefer static linkage" ON)

set(CACOS_CONFIGS_PREFIX "${CMAKE_INSTALL_PREFIX}/share/" CACHE PATH "Where to install the default configs")
set(CACOS_VERSION "0.1.0" CACHE "" INTERNAL)

# Sanitizers build type

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(Sanitizers)

# Debug
set(CACOS_DEBUG OFF)
if (CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "Sanitizers")
    set(CACOS_DEBUG ON)
endif()

# Detect platform

set(CACOS_OS "CACOS_OS_UNDEFINED")
if (WIN32)
    set(CACOS_OS "CACOS_OS_WINDOWS")
elseif (APPLE)
    set(CACOS_OS "CACOS_OS_MACOS")
elseif (UNIX)
    set(CACOS_OS "CACOS_OS_UNIX")
endif()

# Sources

file(GLOB SOURCES
    src/config/config.cpp

    src/commands/init.cpp
    src/commands/config.cpp

    src/executable/executable.cpp
    src/executable/flags.cpp

    src/test/test.cpp

    src/test/add/add.cpp
    src/test/generate/generate.cpp
    src/test/generate/generator.cpp

    src/process/unix/info.cpp
    src/process/process.cpp
    src/process/result.cpp

    src/lang/compiler.cpp
    src/lang/interpreter.cpp
    src/lang/lang.cpp
    src/lang/translator.cpp

    src/ejudge/html/myhtml.cpp
    src/ejudge/http/client.cpp
    src/ejudge/parser/parser.cpp
    src/ejudge/session.cpp
    src/ejudge/solution.cpp
    src/ejudge/status.cpp

    src/util/inline_variables.cpp
    src/util/logger.cpp
    src/util/split.cpp

    src/util/mt/fixed_queue.cpp

    src/cacos.cpp
)

file(GLOB HEADERS
    include/cacos/options.h

    include/cacos/config/config.h

    include/cacos/commands/init.h
    include/cacos/commands/config.h

    include/cacos/executable/executable.h
    include/cacos/executable/flags.h

    include/cacos/test/test.h
    include/cacos/test/add.h
    include/cacos/test/generate/generate.h
    include/cacos/test/generate/generator.h

    include/cacos/process/info.h
    include/cacos/process/limits.h
    include/cacos/process/process.h
    include/cacos/process/result.h

    include/cacos/lang/compiler.h
    include/cacos/lang/interpreter.h
    include/cacos/lang/lang.h
    include/cacos/lang/opts.h
    include/cacos/lang/translator.h

    include/cacos/ejudge/html/myhtml.h
    include/cacos/ejudge/http/client.h
    include/cacos/ejudge/parser/parser.h
    include/cacos/ejudge/parser/task.h
    include/cacos/ejudge/opts.h
    include/cacos/ejudge/session.h
    include/cacos/ejudge/solution.h
    include/cacos/ejudge/status.h

    include/cacos/util/inline_variables.h
    include/cacos/util/logger.h
    include/cacos/util/ranges.h
    include/cacos/util/split.h
    include/cacos/util/string.h
    include/cacos/util/util.h
    include/cacos/util/ints.h

    include/cacos/util/mt/fixed_queue.h

    include/cacos.h
)

file(GLOB CONFIGS
    cacos.toml
    langs.toml
)

set(CACOS_LIB ${PROJECT_NAME}_lib)

add_library(${CACOS_LIB} STATIC ${HEADERS} ${SOURCES} ${CONFIGS})

# Dependecies

set(CACOS_LIBS)

## Threads
find_package(Threads REQUIRED)
list(APPEND CACOS_LIBS ${CMAKE_THREAD_LIBS_INIT})


## CURL
find_package(CURL)
if (${CURL_FOUND})
    list(APPEND CACOS_LIBS CURL::libcurl)
else()
    message(STATUS " CURL was not found; compiling without its support")
endif()

## Boost
set(Boost_USE_MULTITHREAD ON)
set(Boost_USE_STATIC_LIBS ON)
if (CACOS_DOWNLOAD_BOOST)
    message(STATUS " Downloading boost")
    add_subdirectory(third-party/boost-cmake)
else()
    find_package(Boost COMPONENTS system thread filesystem REQUIRED)
endif()
list(APPEND CACOS_LIBS Boost::boost Boost::filesystem)

## cpptoml
set(ENABLE_LIBCXX OFF CACHE "" INTERNAL)
set(CPPTOML_BUILD_EXAMPLES OFF CACHE "" INTERNAL)
add_subdirectory(third-party/toml EXCLUDE_FROM_ALL)
list(APPEND CACOS_LIBS cpptoml)

## cpparg
add_subdirectory(third-party/cpparg EXCLUDE_FROM_ALL)
list(APPEND CACOS_LIBS cpparg)

## termcolor
add_subdirectory(third-party/termcolor EXCLUDE_FROM_ALL)
list(APPEND CACOS_LIBS termcolor::termcolor)

## fmt
add_subdirectory(third-party/fmt EXCLUDE_FROM_ALL)
list(APPEND CACOS_LIBS fmt::fmt)

## dtl
add_subdirectory(third-party/dtl EXCLUDE_FROM_ALL)
list(APPEND CACOS_LIBS dtl::dtl)

## myhtml
add_subdirectory(third-party/myhtml EXCLUDE_FROM_ALL)
list(APPEND CACOS_LIBS myhtml_static)

target_compile_definitions(
    ${CACOS_LIB}
    PUBLIC
    CACOS_CONFIG_PREFIX="${CACOS_CONFIGS_PREFIX}"
    CACOS_VERSION="${CACOS_VERSION}"
    CACOS_HAS_CURL=$<BOOL:${CURL_FOUND}>
    CACOS_DEBUG=$<BOOL:${CACOS_DEBUG}>
    ${CACOS_OS}=1
)

set(CXX_FILESYSTEM_LIBRARY stdc++fs)
list(APPEND CACOS_LIBS ${CXX_FILESYSTEM_LIBRARY})

target_include_directories(${CACOS_LIB} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include/
    ${MYHTML_INCLUDE_DIRS}
)

target_link_libraries(${CACOS_LIB} PUBLIC ${CACOS_LIBS})

# Executable

add_executable(${PROJECT_NAME} src/main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE ${CACOS_LIB})


# LTO

cmake_policy(SET CMP0069 NEW)
include(CheckIPOSupported)
check_ipo_supported(RESULT result OUTPUT output)
if(result)
    set_property(TARGET ${PROJECT_NAME} PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
else()
    message(WARNING "LPO is not supported: ${output}")
endif()


# Installation

install(
    TARGETS ${PROJECT_NAME}
    COMPONENT ${PROJECT_NAME}
    DESTINATION bin
)

install(
    FILES ${CONFIGS}
    DESTINATION ${CACOS_CONFIGS_PREFIX}/cacos/
    COMPONENT configs
)

# Uninstall target
if(NOT TARGET uninstall)
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
        IMMEDIATE @ONLY)

    add_custom_target(uninstall
        COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
endif()


if (${CACOS_BUILD_TESTS})
    enable_testing()
    add_subdirectory(test)
endif()
