cmake_minimum_required(VERSION 3.8)

project(cacos)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_STATIC_LIBS "Prefer static linkage" ON)
option(CACOS_USE_CONAN "Use conan" OFF)
option(CACOS_BUILD_TESTS "Build tests" OFF)

set(CACOS_VERSION "0.1.8" CACHE "" INTERNAL)

# Modules
list(APPEND CMAKE_MODULE_PATH
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake
    ${CMAKE_CURRENT_SOURCE_DIR}/third-party/cmake-git-version-tracking
    ${CMAKE_CURRENT_SOURCE_DIR}/third-party/cmake-conan)

# Output binary dir
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Sanitizers build type
include(Sanitizers)

# Debug
set(CACOS_DEBUG OFF)
if (CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "Sanitizers")
    set(CACOS_DEBUG ON)
endif()

# Detect platform
set(CACOS_OS "CACOS_OS_UNDEFINED")
if (WIN32 OR WIN64)
    set(CACOS_OS "CACOS_OS_WINDOWS")
elseif (APPLE)
    set(CACOS_OS "CACOS_OS_MACOS")
elseif (UNIX)
    set(CACOS_OS "CACOS_OS_UNIX")
endif()

# Static CRT
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    string(REPLACE "/MD" "/MT" CMAKE_CXX_FLAGS_DEBUG ${CMAKE_CXX_FLAGS_DEBUG})
    string(REPLACE "/MD" "/MT" CMAKE_C_FLAGS_DEBUG ${CMAKE_C_FLAGS_DEBUG})
    string(REPLACE "/MD" "/MT" CMAKE_CXX_FLAGS_RELEASE ${CMAKE_CXX_FLAGS_RELEASE})
    string(REPLACE "/MD" "/MT" CMAKE_C_FLAGS_RELEASE ${CMAKE_C_FLAGS_RELEASE})
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
    src/test/suite/suite.cpp
    src/test/suite/test.cpp
    src/test/generate/generate.cpp
    src/test/generate/generator.cpp
    src/test/run/run.cpp

    src/process/unix/info.cpp
    src/process/windows/info.cpp
    src/process/process.cpp
    src/process/result.cpp

    src/lang/compiler.cpp
    src/lang/interpreter.cpp
    src/lang/lang.cpp
    src/lang/linker.cpp
    src/lang/translator.cpp

    src/ejudge/diff.cpp
    src/ejudge/html/myhtml.cpp
    src/ejudge/html/printer.cpp
    src/ejudge/http/client.cpp
    src/ejudge/parser/parser.cpp
    src/ejudge/session.cpp
    src/ejudge/status.cpp
    src/ejudge/task.cpp

    src/version/version.cpp

    src/task/opts.cpp

    src/util/inline_variables.cpp
    src/util/logger.cpp
    src/util/map.cpp
    src/util/optional_ref.cpp
    src/util/progress_bar.cpp
    src/util/split.cpp
    src/util/string.cpp
    src/util/temp_directory.cpp

    src/util/terminfo/posix.cpp
    src/util/terminfo/windows.cpp
    src/util/terminfo/default.cpp

    src/util/diff/unified.cpp

    src/util/mt/fixed_queue.cpp

    src/cacos.cpp
)

file(GLOB HEADERS
    include/cacos/options.h

    include/cacos/config/config.h
    include/cacos/config/default.h

    include/cacos/commands/init.h
    include/cacos/commands/config.h

    include/cacos/executable/executable.h
    include/cacos/executable/flags.h

    include/cacos/test/test.h
    include/cacos/test/add.h
    include/cacos/test/suite/suite.h
    include/cacos/test/suite/test.h
    include/cacos/test/generate/generate.h
    include/cacos/test/generate/generator.h
    include/cacos/test/run/run.h

    include/cacos/process/info.h
    include/cacos/process/limits.h
    include/cacos/process/process.h
    include/cacos/process/result.h

    include/cacos/lang/compiler.h
    include/cacos/lang/interpreter.h
    include/cacos/lang/lang.h
    include/cacos/lang/linker.h
    include/cacos/lang/opts.h
    include/cacos/lang/translator.h

    include/cacos/ejudge/diff.h
    include/cacos/ejudge/html/myhtml.h
    include/cacos/ejudge/html/printer.h
    include/cacos/ejudge/http/client.h
    include/cacos/ejudge/opts.h
    include/cacos/ejudge/parser/parser.h
    include/cacos/ejudge/parser/task.h
    include/cacos/ejudge/session.h
    include/cacos/ejudge/status.h
    include/cacos/ejudge/task.h

    include/cacos/version/git.h
    include/cacos/version/id.h
    include/cacos/version/version.h

    include/cacos/task/opts.h

    include/cacos/util/inline_variables.h
    include/cacos/util/ints.h
    include/cacos/util/logger.h
    include/cacos/util/map.h
    include/cacos/util/optional_ref.h
    include/cacos/util/progress_bar.h
    include/cacos/util/ranges.h
    include/cacos/util/split.h
    include/cacos/util/string.h
    include/cacos/util/temp_directory.h
    include/cacos/util/terminfo/posix.h
    include/cacos/util/terminfo/terminfo.h
    include/cacos/util/util.h

    include/cacos/util/diff/unified.h

    include/cacos/util/mt/fixed_queue.h

    include/cacos.h
)

file(GLOB CONFIGS
    config/cacos.toml
    config/langs.toml
    config/task.toml
)

file(GLOB CONFIGURABLE_SOURCES
    src/config/default.cpp.in
    src/version/git.cpp.in
    src/version/id.cpp.in
)

# Save configs to the binary
foreach(config ${CONFIGS})
    get_filename_component(sourcefile ${config} NAME)
    file(READ ${config} CACOS_CONFIG_FILE_${sourcefile})
endforeach()

set(CONFIGS_PRE_CONFIGURE ${CMAKE_CURRENT_SOURCE_DIR}/src/config/default.cpp.in)
set(CONFIGS_POST_CONFIGURE ${CMAKE_CURRENT_BINARY_DIR}/src/default.cpp)
configure_file(${CONFIGS_PRE_CONFIGURE} ${CONFIGS_POST_CONFIGURE} @ONLY)
list(APPEND SOURCES ${CONFIGS_POST_CONFIGURE})

string(TIMESTAMP BUILD_TIMESTAMP)
set(BUILD_ID_PRE_CONFIGURE ${CMAKE_CURRENT_SOURCE_DIR}/src/version/id.cpp.in)
set(BUILD_ID_CONFIGURE ${CMAKE_CURRENT_BINARY_DIR}/src/id.cpp)
configure_file(${BUILD_ID_PRE_CONFIGURE} ${BUILD_ID_CONFIGURE} @ONLY)
list(APPEND SOURCES ${BUILD_ID_CONFIGURE})

# Include git version
set(_BUILD_TIME_CHECK_GIT ON)
set(PRE_CONFIGURE_FILE ${CMAKE_CURRENT_SOURCE_DIR}/src/version/git.cpp.in)
set(POST_CONFIGURE_FILE ${CMAKE_CURRENT_BINARY_DIR}/src/git.cpp)
include(git_watcher)
list(APPEND SOURCES ${POST_CONFIGURE_FILE})

set(CACOS_LIB ${PROJECT_NAME}_lib)

add_library(${CACOS_LIB} STATIC ${HEADERS} ${SOURCES} ${CONFIGS} ${CONFIGURABLE_SOURCES})

# Dependecies

set(CACOS_LIBS)
set(CACOS_INCLUDES)
list(APPEND CACOS_INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/include/)

if(${CACOS_USE_CONAN})
    set(CACHED_BUILD_TYPE ${CMAKE_BUILD_TYPE})
    if ("${CMAKE_BUILD_TYPE}" STREQUAL Sanitizers)
        set(CMAKE_BUILD_TYPE Debug)
    endif()
    include(conan)
    conan_cmake_run(CONANFILE conanfile.txt BASIC_SETUP CMAKE_TARGETS BUILD missing)
    set(CMAKE_BUILD_TYPE ${CACHED_BUILD_TYPE})
    list(APPEND CACOS_LIBS CONAN_PKG::boost CONAN_PKG::libcurl)
else()
    ## CURL
    find_package(CURL)
    if (${CURL_FOUND})
        list(APPEND CACOS_LIBS ${CURL_LIBRARIES})
        list(APPEND CACOS_INCLUDES ${CURL_INCLUDE_DIRS})
    else()
        message(STATUS " libcurl was not found; compiling without ejudge client")
    endif()

    ## Boost
    set(Boost_USE_MULTITHREAD ON)
    set(Boost_USE_STATIC_LIBS ${BUILD_STATIC_LIBS})
    find_package(Boost COMPONENTS system thread filesystem REQUIRED)
    ## 1.69 and 1.70 have problems with leaking file descriptors
    if (Boost_MAJOR_VERSION EQUAL 1 AND Boost_MINOR_VERSION GREATER 68)
        message(FATAL_ERROR "Boost versions 1.69 and 1.70 are not supported")
    endif()

    list(APPEND CACOS_LIBS Boost::boost Boost::filesystem)
endif()

if (UNIX)
    list(APPEND CACOS_LIBS ${CMAKE_DL_LIBS})
endif()

## Threads
find_package(Threads REQUIRED)
list(APPEND CACOS_LIBS ${CMAKE_THREAD_LIBS_INIT})

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
find_package(Threads)
set(MyHTML_BUILD_WITHOUT_THREADS ON CACHE BOOL "" FORCE)
set(MyCORE_BUILD_WITHOUT_THREADS ON CACHE BOOL "" FORCE)
if (CMAKE_USE_PTHREADS_INIT)
    message(STATUS "Found pthread")
    set(MyHTML_BUILD_WITHOUT_THREADS OFF CACHE BOOL "" FORCE)
    set(MyCORE_BUILD_WITHOUT_THREADS OFF CACHE BOOL "" FORCE)
else()
    message(STATUS "pthread was not found")
endif()
add_subdirectory(third-party/myhtml EXCLUDE_FROM_ALL)
list(APPEND CACOS_LIBS myhtml_static)
list(APPEND CACOS_INCLUDES ${MYHTML_INCLUDE_DIRS})

## utfcpp
set(UTF8_TESTS OFF CACHE BOOL "" FORCE)
set(UTF8_SAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(third-party/utfcpp EXCLUDE_FROM_ALL)
list(APPEND CACOS_LIBS utf8::cpp)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    target_compile_definitions(
    ${CACOS_LIB}
    PUBLIC
    _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS
    _WIN32_WINNT=0x0601)
    target_compile_options(${CACOS_LIB} PUBLIC /utf-8)

    ### Use all cores
    include(ProcessorCount)
    ProcessorCount(NPROC)
    if(NOT NPROC EQUAL 0)
        target_compile_options(${CACOS_LIB} PUBLIC /utf-8 "/MP${NPROC}")
    endif()
endif()

## Warnings
if (${CMAKE_CXX_COMPILER_ID} MATCHES "Clang" OR ${CMAKE_CXX_COMPILER_ID} MATCHES "GNU")
    target_compile_options(${CACOS_LIB} PUBLIC -Wall -Wextra -Wpedantic)
elseif (${CMAKE_CXX_COMPILER_ID} MATCHES "MSVC")
    target_compile_options(${CACOS_LIB} PUBLIC /W4)
endif()

target_compile_definitions(
    ${CACOS_LIB}
    PUBLIC
    CACOS_VERSION="${CACOS_VERSION}"
    CACOS_HAS_CURL=$<BOOL:${CURL_FOUND}>
    CACOS_DEBUG=$<BOOL:${CACOS_DEBUG}>
    ${CACOS_OS}=1
)

## std::filesystem
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    set(CXX_FILESYSTEM_LIBRARY stdc++fs)
    list(APPEND CACOS_LIBS ${CXX_FILESYSTEM_LIBRARY})
endif()

target_link_libraries(${CACOS_LIB} PUBLIC ${CACOS_LIBS})
target_include_directories(${CACOS_LIB} PUBLIC ${CACOS_INCLUDES})

# Executable

if(POLICY CMP0069)
    cmake_policy(SET CMP0069 NEW)
endif()
add_executable(${PROJECT_NAME} src/main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE ${CACOS_LIB})


# LTO
if(CMAKE_VERSION VERSION_LESS "3.9.0")
    message(STATUS "Old cmake version (${CMAKE_VERSION}); LTO is not supported.")
else()
    include(CheckIPOSupported)
    check_ipo_supported(RESULT result OUTPUT output)
    if(result)
        set_property(TARGET ${PROJECT_NAME} PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
    else()
        message(WARNING "LTO is not supported: ${output}")
    endif()
endif()

# Installation

install(
    TARGETS ${PROJECT_NAME}
    COMPONENT ${PROJECT_NAME}
    DESTINATION bin
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
    add_subdirectory(test/unit)
endif()
