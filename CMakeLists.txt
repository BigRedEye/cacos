cmake_minimum_required(VERSION 3.5)

project(cacos)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(CACOS_BUILD_TESTS "Build tests" OFF)

set(CACOS_CONFIGS_PREFIX "/etc/" CACHE PATH "Where to place the default configs")
set(CACOS_VERSION "0.1.0" CACHE "" INTERNAL)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(Sanitizers)

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
SET(Boost_USE_MULTITHREAD ON)
find_package(Boost REQUIRED COMPONENTS system thread)

# cpptoml
set(ENABLE_LIBCXX OFF CACHE "" INTERNAL)
set(CPPTOML_BUILD_EXAMPLES OFF CACHE "" INTERNAL)

add_subdirectory(third-party/toml EXCLUDE_FROM_ALL)

# cpparg
add_subdirectory(third-party/cpparg EXCLUDE_FROM_ALL)

# termcolor
add_subdirectory(third-party/termcolor EXCLUDE_FROM_ALL)

target_compile_definitions(
    ${CACOS_LIB}
    PUBLIC
    DEFAULT_CONFIG_PREFIX="${CACOS_CONFIGS_PREFIX}"
    CACOS_VERSION="${CACOS_VERSION}"
    CACOS_HAS_CURL=$<BOOL:${CURL_FOUND}>
)

set(CXX_FILESYSTEM_LIBRARY stdc++fs)

target_include_directories(${CACOS_LIB} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include/
    ${Boost_INCLUDE_DIRS}
)

target_link_libraries(${CACOS_LIB} PUBLIC
    cpparg
    termcolor::termcolor
    cpptoml
    ${Boost_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
    ${CXX_FILESYSTEM_LIBRARY}
)

# Executable

add_executable(${PROJECT_NAME} src/main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE ${CACOS_LIB})

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
