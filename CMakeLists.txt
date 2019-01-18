cmake_minimum_required(VERSION 3.12)

project(cacos)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(CACOS_BUILD_TESTS "Build tests" OFF)

set(CACOS_CONFIGS_PREFIX "${CMAKE_INSTALL_PREFIX}/share/" CACHE PATH "Where to place the default configs")
set(CACOS_VERSION "0.1.0" CACHE "" INTERNAL)

# Sanitizers build type

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(Sanitizers)

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
    src/common_args.cpp
    src/config.cpp
    src/init.cpp
    src/language.cpp
    src/options.cpp

    src/executable/executable.cpp
    src/executable/flags.cpp

    src/test/test.cpp

    src/test/add/add.cpp
    src/test/generate/generate.cpp
    src/test/generate/generator.cpp

    src/process/unix/process.cpp
    src/process/result.cpp

    src/lang/compiler.cpp
    src/lang/interpreter.cpp
    src/lang/lang.cpp
    src/lang/translator.cpp

    src/util/inline_variables.cpp
    src/util/logger.cpp
    src/util/split.cpp

    src/util/mt/fixed_queue.cpp

    src/cacos.cpp
)

file(GLOB HEADERS
    include/cacos/config.h
    include/cacos/common_args.h
    include/cacos/language.h
    include/cacos/init.h
    include/cacos/options.h

    include/cacos/executable/executable.h
    include/cacos/executable/flags.h

    include/cacos/test/test.h
    include/cacos/test/add.h
    include/cacos/test/generate/generate.h
    include/cacos/test/generate/generator.h

    include/cacos/process/limits.h
    include/cacos/process/process.h
    include/cacos/process/result.h

    include/cacos/lang/lang.h
    include/cacos/lang/compiler.h
    include/cacos/lang/interpreter.h
    include/cacos/lang/translator.h

    include/cacos/util/inline_variables.h
    include/cacos/util/logger.h
    include/cacos/util/split.h
    include/cacos/util/string.h
    include/cacos/util/util.h

    include/cacos/util/mt/fixed_queue.h

    include/cacos.h
)

file(GLOB CONFIGS
    langs.toml
)

set(CACOS_LIB ${PROJECT_NAME}_lib)

add_library(${CACOS_LIB} STATIC ${HEADERS} ${SOURCES} ${CONFIGS})

# Dependecies

# Threads
find_package(Threads REQUIRED)

# CURL
find_package(CURL)
if (${CURL_FOUND})
    target_link_libraries(${CACOS_LIB} PUBLIC CURL::libcurl)
else()
    message(STATUS " CURL was not found; compiling without its support")
endif()


# Boost
set(Boost_USE_MULTITHREAD ON)
set(Boost_USE_STATIC_LIBS ON)
find_package(Boost COMPONENTS system thread filesystem)
if (NOT Boost_FOUND)
    # download boost
    add_subdirectory(third-party/boost-cmake)
endif()


# cpptoml
set(ENABLE_LIBCXX OFF CACHE "" INTERNAL)
set(CPPTOML_BUILD_EXAMPLES OFF CACHE "" INTERNAL)
add_subdirectory(third-party/toml EXCLUDE_FROM_ALL)

# cpparg
add_subdirectory(third-party/cpparg EXCLUDE_FROM_ALL)

# termcolor
add_subdirectory(third-party/termcolor EXCLUDE_FROM_ALL)

# fmt
add_subdirectory(third-party/fmt EXCLUDE_FROM_ALL)

target_compile_definitions(
    ${CACOS_LIB}
    PUBLIC
    CACOS_CONFIG_PREFIX="${CACOS_CONFIGS_PREFIX}"
    CACOS_VERSION="${CACOS_VERSION}"
    CACOS_HAS_CURL=$<BOOL:${CURL_FOUND}>
    ${CACOS_OS}=1
)

set(CXX_FILESYSTEM_LIBRARY stdc++fs)

target_include_directories(${CACOS_LIB} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include/
    ${Boost_INCLUDE_DIRS}
)

target_link_libraries(${CACOS_LIB} PUBLIC
    cpparg
    termcolor::termcolor
    fmt::fmt
    Boost::boost
    Boost::filesystem
    cpptoml
    ${CMAKE_THREAD_LIBS_INIT}
    ${CXX_FILESYSTEM_LIBRARY}
)

# Executable

add_executable(${PROJECT_NAME} src/main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE ${CACOS_LIB})

# LTO

cmake_policy(SET CMP0069 NEW)
include(CheckIPOSupported)
check_ipo_supported(RESULT result OUTPUT output)
if(result)
  set_property(TARGET ${PROJECT_NAME} PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
  set_property(TARGET ${CACOS_LIB} PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
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

if (${CACOS_BUILD_TESTS})
    enable_testing()
    add_subdirectory(test)
endif()
