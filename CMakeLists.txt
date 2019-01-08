cmake_minimum_required(VERSION 3.5)

project(cacos)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CACOS_CONFIGS_PREFIX "/etc/" CACHE PATH "Where to place the default configs")
set(CACOS_VERSION "0.1.0" CACHE "" INTERNAL)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(Sanitizers)

file(GLOB SOURCES
    src/config.cpp
    src/common_args.cpp
    src/language.cpp
    src/init.cpp
    src/main.cpp
    src/test.cpp
    src/options.cpp
    src/executable/executable.cpp
    src/executable/flags.cpp
)

file(GLOB HEADERS
    include/cacos/config.h
    include/cacos/common_args.h
    include/cacos/language.h
    include/cacos/init.h
    include/cacos/test.h
    include/cacos/util.h
    include/cacos/options.h
    include/cacos/executable/executable.h
    include/cacos/executable/flags.h
)

file(GLOB CONFIGS
    langs.toml
)

add_executable(${PROJECT_NAME})

target_sources(${PROJECT_NAME} PUBLIC
    ${HEADERS} ${SOURCES} ${CONFIGS}
)


find_package(Threads)

SET(Boost_USE_MULTITHREAD ON)
find_package(Boost REQUIRED COMPONENTS system thread)

set(ENABLE_LIBCXX OFF CACHE "" INTERNAL)
set(CPPTOML_BUILD_EXAMPLES OFF CACHE "" INTERNAL)

add_subdirectory(third-party/cpparg EXCLUDE_FROM_ALL)
add_subdirectory(third-party/termcolor EXCLUDE_FROM_ALL)
add_subdirectory(third-party/toml EXCLUDE_FROM_ALL)
target_compile_definitions(
    ${PROJECT_NAME}
    PRIVATE
    DEFAULT_CONFIG_PREFIX="${CACOS_CONFIGS_PREFIX}"
    CACOS_VERSION="${CACOS_VERSION}"
)

set(CXX_FILESYSTEM_LIBRARY stdc++fs)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include/
    ${Boost_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    cpparg
    termcolor::termcolor
    cpptoml
    ${Boost_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
    ${CXX_FILESYSTEM_LIBRARY}
)

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

