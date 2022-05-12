# Custom cmake configuration only to build the CSV parser portion of orcus.

cmake_minimum_required(VERSION 2.8)
cmake_policy(SET CMP0048 NEW)

set(ORCUS_MAJOR_VERSION 0)
set(ORCUS_MINOR_VERSION 13)
set(ORCUS_MICRO_VERSION 99)
set(ORCUS_VERSION ${ORCUS_MAJOR_VERSION}.${ORCUS_MINOR_VERSION}.${ORCUS_MICRO_VERSION})

project(orcus VERSION ${ORCUS_VERSION} LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 11)

if(MSVC)
    set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} /wd4251 /wd4275 /MP")
endif(MSVC)

include_directories(
    ${PROJECT_SOURCE_DIR}/../include
)

set(PARSER_SRC_DIR ${PROJECT_SOURCE_DIR}/../src/parser)

add_library(liborcus-parser-csv SHARED
    ${PARSER_SRC_DIR}/parser_base.cpp
    ${PARSER_SRC_DIR}/parser_global.cpp
    ${PARSER_SRC_DIR}/csv_parser_base.cpp
    ${PARSER_SRC_DIR}/exception.cpp
    ${PARSER_SRC_DIR}/cell_buffer.cpp
)

target_compile_definitions(liborcus-parser-csv PRIVATE __ORCUS_PSR_BUILDING_DLL)

