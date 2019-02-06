cmake_minimum_required(VERSION 3.12)

project(cacos)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_STATIC_LIBS "Prefer static linkage" ON)
option(CACOS_USE_CONAN "Use conan" OFF)
option(CACOS_BUILD_TESTS "Build tests" OFF)

set(CACOS_VERSION "0.1.0" CACHE "" INTERNAL)

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
    src/process/windows/info.cpp
    src/process/process.cpp
    src/process/result.cpp

    src/lang/compiler.cpp
    src/lang/interpreter.cpp
    src/lang/lang.cpp
    src/lang/linker.cpp
    src/lang/translator.cpp

    src/ejudge/html/myhtml.cpp
    src/ejudge/http/client.cpp
    src/ejudge/parser/parser.cpp
    src/ejudge/session.cpp
    src/ejudge/solution.cpp
    src/ejudge/status.cpp

    src/version/version.cpp

    src/task/opts.cpp

    src/util/inline_variables.cpp
    src/util/logger.cpp
    src/util/optional_ref.cpp
    src/util/split.cpp

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
    include/cacos/test/generate/generate.h
    include/cacos/test/generate/generator.h

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

    include/cacos/ejudge/html/myhtml.h
    include/cacos/ejudge/http/client.h
    include/cacos/ejudge/parser/parser.h
    include/cacos/ejudge/parser/task.h
    include/cacos/ejudge/opts.h
    include/cacos/ejudge/session.h
    include/cacos/ejudge/solution.h
    include/cacos/ejudge/status.h

    include/cacos/version/git.h
    include/cacos/version/version.h

    include/cacos/task/opts.h

    include/cacos/util/inline_variables.h
    include/cacos/util/ints.h
    include/cacos/util/logger.h
    include/cacos/util/optional_ref.h
    include/cacos/util/ranges.h
    include/cacos/util/split.h
    include/cacos/util/string.h
    include/cacos/util/util.h

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
        list(APPEND CACOS_LIBS CURL::libcurl)
    else()
        message(STATUS " libcurl was not found; compiling without ejudge client")
    endif()

    ## Boost
    set(Boost_USE_MULTITHREAD ON)
    set(Boost_USE_STATIC_LIBS ON)
    find_package(Boost COMPONENTS system thread filesystem REQUIRED)
    list(APPEND CACOS_LIBS Boost::boost Boost::filesystem)
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
set(MyHTML_BUILD_WITHOUT_THREADS OFF)
set(MyCORE_BUILD_WITHOUT_THREADS OFF)
if (WIN32)
	set(MyHTML_BUILD_WITHOUT_THREADS ON)
	set(MyCORE_BUILD_WITHOUT_THREADS ON)
endif(WIN32)
add_subdirectory(third-party/myhtml EXCLUDE_FROM_ALL)
list(APPEND CACOS_LIBS myhtml_static)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    target_compile_definitions(
    ${CACOS_LIB}
    PUBLIC
    _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS
    _WIN32_WINNT=0x0601)
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

target_include_directories(${CACOS_LIB} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include/
    ${MYHTML_INCLUDE_DIRS}
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
else()
    message(WARNING "LTO is not supported: ${output}")
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
    add_subdirectory(test)
endif()
