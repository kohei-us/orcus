cmake_minimum_required(VERSION 3.12)
cmake_policy(SET CMP0048 NEW)

set(ORCUS_MAJOR_VERSION 0)
set(ORCUS_MINOR_VERSION 20)
set(ORCUS_MICRO_VERSION 0)
set(ORCUS_MAJOR_API_VERSION 0)
set(ORCUS_MINOR_API_VERSION 20)
set(ORCUS_VERSION ${ORCUS_MAJOR_VERSION}.${ORCUS_MINOR_VERSION}.${ORCUS_MICRO_VERSION})
set(ORCUS_API_VERSION ${ORCUS_MAJOR_API_VERSION}.${ORCUS_MINOR_API_VERSION})

project(orcus VERSION ${ORCUS_VERSION} LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)

set(MDDS_INCLUDEDIR "" CACHE STRING "path to mdds header directory.")
set(IXION_INCLUDEDIR "" CACHE STRING "path to ixion header directory.")
set(IXION_LIBRARYDIR "" CACHE STRING "path to ixion library directory.")
option(ORCUS_WITH_CPU_FEATURES "whether or not to enable use of CPU extensions.")
option(ORCUS_WITH_SSE42 "whether or not to enable use of SSE 4.2.")
option(ORCUS_WITH_AVX2 "whether or not to enable use of AVX2.")
option(ORCUS_WITH_PARQUET "whether or not to enable support for Apache Parquet import filter.")

find_package(Boost COMPONENTS program_options filesystem)
find_package(Threads)
find_package(Python3)
find_package(ZLIB)

include(GNUInstallDirs)

enable_testing()
set(CMAKE_CTEST_COMMAND ctest)

add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND})

find_library(IXION_LIB ixion-0.20 PATHS ${IXION_LIBRARYDIR})

if(NOT IXION_LIB)
    message(FATAL_ERROR "Ixion not found.")
endif()

message(STATUS "Found Ixion: ${IXION_LIB}")

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4251 /wd4275 /wd4267 /wd4244 /MP")
    add_definitions(
        -D_CRT_SECURE_NO_WARNINGS
    )
endif(MSVC)

add_definitions(
    ${Boost_LIB_DIAGNOSTIC_DEFINITIONS}
    -D__ORCUS_SPREADSHEET_MODEL
)

if(ORCUS_WITH_CPU_FEATURES)
    add_definitions(-D__ORCUS_CPU_FEATURES)
    message(STATUS "Enabling use of CPU features...")

    if(ORCUS_WITH_AVX2)
        message(STATUS "Enabling AVX2...")
        if(MSVC)
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /arch:AVX2")
        else()
            add_definitions(-D__AVX2__)
        endif()
    endif()

    if(ORCUS_WITH_SSE42)
        message(STATUS "Enabling SSE 4.2...")
        add_definitions(-D__SSE4_2__)
    endif()

endif()

if(ORCUS_WITH_PARQUET)
    find_package(Parquet)
endif()

include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/src/include
    ${Boost_INCLUDE_DIR}
    ${MDDS_INCLUDEDIR}
    ${IXION_INCLUDEDIR}
    ${ZLIB_INCLUDE_DIRS}
)

link_directories(${Boost_LIBRARY_DIRS})

add_subdirectory(include)
add_subdirectory(src)
