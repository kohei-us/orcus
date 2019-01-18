cmake_minimum_required(VERSION 2.8)
cmake_policy(SET CMP0048 NEW)

set(ORCUS_MAJOR_VERSION 0)
set(ORCUS_MINOR_VERSION 14)
set(ORCUS_MICRO_VERSION 99)
set(ORCUS_VERSION ${ORCUS_MAJOR_VERSION}.${ORCUS_MINOR_VERSION}.${ORCUS_MICRO_VERSION})

project(orcus VERSION ${ORCUS_VERSION} LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 11)

option(MDDS_INCLUDEDIR "path to mdds header directory.")
option(IXION_INCLUDEDIR "path to ixion header directory.")
option(IXION_LIBRARYDIR "path to ixion library directory.")

set(Boost_USE_STATIC_LIBS 1)
find_package(Boost COMPONENTS program_options filesystem)
find_package(Threads)
find_package(Python3)
find_package(Zlib)

enable_testing()
set(CMAKE_CTEST_COMMAND ctest -V)

add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND})

find_library(IXION_LIB libixion PATHS ${IXION_LIBRARYDIR})

if(NOT IXION_LIB)
    message(FATAL_ERROR "Ixion not found.")
endif()

message(STATUS "Found Ixion: ${IXION_LIB}")

if(MSVC)
    set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} /wd4251 /wd4275 /wd4267 /wd4244 /MP")
    add_definitions(
        -D_CRT_SECURE_NO_WARNINGS
    )
endif(MSVC)

add_definitions(
    ${Boost_LIB_DIAGNOSTIC_DEFINITIONS}
)

include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/src/include
    ${Boost_INCLUDE_DIR}
    ${MDDS_INCLUDEDIR}
    ${IXION_INCLUDEDIR}
    ${ZLIB_INCLUDE_DIRS}
)

link_directories(${Boost_LIBRARY_DIRS})

add_subdirectory(src)
