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

add_library(liborcus SHARED
	${LIBORCUS_SRC_DIR}/config.cpp
	${LIBORCUS_SRC_DIR}/css_document_tree.cpp
	${LIBORCUS_SRC_DIR}/css_selector.cpp
	${LIBORCUS_SRC_DIR}/detection_result.cpp
	${LIBORCUS_SRC_DIR}/dom_tree.cpp
	${LIBORCUS_SRC_DIR}/format_detection.cpp
	${LIBORCUS_SRC_DIR}/global.cpp
	${LIBORCUS_SRC_DIR}/info.cpp
	${LIBORCUS_SRC_DIR}/interface.cpp
	${LIBORCUS_SRC_DIR}/json_document_tree.cpp
	${LIBORCUS_SRC_DIR}/json_util.cpp
	${LIBORCUS_SRC_DIR}/spreadsheet_interface.cpp
	${LIBORCUS_SRC_DIR}/orcus_csv.cpp
	${LIBORCUS_SRC_DIR}/orcus_xml.cpp
	${LIBORCUS_SRC_DIR}/measurement.cpp
	${LIBORCUS_SRC_DIR}/xml_context_base.cpp
	${LIBORCUS_SRC_DIR}/xml_context_global.cpp
	${LIBORCUS_SRC_DIR}/xml_map_tree.cpp
	${LIBORCUS_SRC_DIR}/xml_stream_handler.cpp
	${LIBORCUS_SRC_DIR}/xml_stream_parser.hpp
	${LIBORCUS_SRC_DIR}/xml_stream_parser.cpp
	${LIBORCUS_SRC_DIR}/xml_simple_stream_handler.cpp
	${LIBORCUS_SRC_DIR}/xml_structure_tree.cpp
	${LIBORCUS_SRC_DIR}/yaml_document_tree.cpp
	${LIBORCUS_SRC_DIR}/ooxml_namespace_types.cpp
	${LIBORCUS_SRC_DIR}/ooxml_namespace_types.hpp
	${LIBORCUS_SRC_DIR}/odf_namespace_types.hpp
	${LIBORCUS_SRC_DIR}/odf_namespace_types.cpp
	${LIBORCUS_SRC_DIR}/gnumeric_namespace_types.cpp
	${LIBORCUS_SRC_DIR}/xls_xml_namespace_types.cpp
	${LIBORCUS_SRC_DIR}/session_context.cpp
	${LIBORCUS_SRC_DIR}/spreadsheet_types.cpp
	${LIBORCUS_SRC_DIR}/string_helper.cpp
)

add_custom_target(
    py_gen_files
    COMMAND python ${PROJECT_SOURCE_DIR}/bin/gen-files.py
        --properties
            ORCUS_MAJOR_VERSION=${ORCUS_MAJOR_VERSION}
            ORCUS_MINOR_VERSION=${ORCUS_MINOR_VERSION}
            ORCUS_MICRO_VERSION=${ORCUS_MICRO_VERSION}
        --files
            ${LIBORCUS_SRC_DIR}/constants.inl
)

add_dependencies(liborcus py_gen_files)

target_link_libraries(liborcus-parser ${ZLIB_LIB})
target_link_libraries(liborcus liborcus-parser)

target_compile_definitions(liborcus-parser PRIVATE __ORCUS_PSR_BUILDING_DLL)
target_compile_definitions(liborcus PRIVATE __ORCUS_BUILDING_DLL)

