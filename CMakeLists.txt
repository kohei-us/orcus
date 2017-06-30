cmake_minimum_required(VERSION 2.8)
cmake_policy(SET CMP0048 NEW)

set(ORCUS_MAJOR_VERSION 0)
set(ORCUS_MINOR_VERSION 12)
set(ORCUS_MICRO_VERSION 99)
set(ORCUS_VERSION ${ORCUS_MAJOR_VERSION}.${ORCUS_MINOR_VERSION}.${ORCUS_MICRO_VERSION})

project(orcus VERSION ${ORCUS_VERSION} LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 11)

option(MDDS_INCLUDE_DIR "path to mdds header directory.")
option(BOOST_INCLUDE_DIR "path to boost header directory.")
option(BOOST_LIB_DIR "path to boost library directory.")
option(IXION_INCLUDE_DIR "path to ixion header directory.")
option(IXION_LIB_DIR "path to ixion library directory.")
option(ZLIB_INCLUDE_DIR "path to zlib header directory.")
option(ZLIB_LIB_DIR "path to zlib library directory.")

include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${BOOST_INCLUDE_DIR}
    ${MDDS_INCLUDE_DIR}
    ${IXION_INCLUDE_DIR}
    ${ZLIB_INCLUDE_DIR}
)

link_directories(${BOOST_LIB_DIR})

find_library(ZLIB_LIB zlib PATHS ${ZLIB_LIB_DIR})

set(PARSER_SRC_DIR ${PROJECT_SOURCE_DIR}/src/parser)
set(LIBORCUS_SRC_DIR ${PROJECT_SOURCE_DIR}/src/liborcus)
set(SPREADSHEET_SRC_DIR ${PROJECT_SOURCE_DIR}/src/spreadsheet)

add_library(liborcus-parser SHARED
	${PARSER_SRC_DIR}/base64.cpp
	${PARSER_SRC_DIR}/cell_buffer.cpp
	${PARSER_SRC_DIR}/css_parser_base.cpp
	${PARSER_SRC_DIR}/css_types.cpp
	${PARSER_SRC_DIR}/csv_parser_base.cpp
	${PARSER_SRC_DIR}/exception.cpp
	${PARSER_SRC_DIR}/json_global.cpp
	${PARSER_SRC_DIR}/json_parser_base.cpp
	${PARSER_SRC_DIR}/json_parser_thread.cpp
	${PARSER_SRC_DIR}/parser_base.cpp
	${PARSER_SRC_DIR}/parser_global.cpp
	${PARSER_SRC_DIR}/pstring.cpp
	${PARSER_SRC_DIR}/sax_parser_base.cpp
	${PARSER_SRC_DIR}/sax_token_parser.cpp
	${PARSER_SRC_DIR}/sax_token_parser_thread.cpp
	${PARSER_SRC_DIR}/stream.cpp
	${PARSER_SRC_DIR}/string_pool.cpp
	${PARSER_SRC_DIR}/tokens.cpp
	${PARSER_SRC_DIR}/types.cpp
	${PARSER_SRC_DIR}/xml_namespace.cpp
	${PARSER_SRC_DIR}/yaml_parser_base.cpp
	${PARSER_SRC_DIR}/zip_archive.cpp
	${PARSER_SRC_DIR}/zip_archive_stream.cpp
)

target_link_libraries(liborcus-parser ${ZLIB_LIB})

target_compile_definitions(liborcus-parser PRIVATE __ORCUS_PSR_BUILDING_DLL)

