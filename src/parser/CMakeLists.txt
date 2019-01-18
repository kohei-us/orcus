
set(PARSER_SRC_DIR ${PROJECT_SOURCE_DIR}/src/parser)

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

target_compile_definitions(liborcus-parser PRIVATE __ORCUS_PSR_BUILDING_DLL)
target_link_libraries(liborcus-parser ${ZLIB_LIBRARIES})

# test programs

set(_TESTS
    base64-test
    parser-base-test
    parser-global-test
#   sax-token-parser-test
    stream_test
    string-pool-test
    threaded-json-parser-test
    threaded-sax-token-parser-test
    xml-namespace-test
    zip-archive-test
)

foreach(_TEST ${_TESTS})
    set(_TEST_FILE "${_TEST}.cpp")
    string(REPLACE "-" "_" _TEST_FILE ${_TEST_FILE})
    add_executable(${_TEST} EXCLUDE_FROM_ALL ${_TEST_FILE})
    target_link_libraries(${_TEST} liborcus-parser)
    add_test(${_TEST} ${_TEST})
endforeach()

add_dependencies(check ${_TESTS})
