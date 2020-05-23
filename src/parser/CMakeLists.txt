
add_library(orcus-parser-${ORCUS_API_VERSION} SHARED
    base64.cpp
	cell_buffer.cpp
	css_parser_base.cpp
	css_types.cpp
	csv_parser_base.cpp
	exception.cpp
	json_global.cpp
	json_parser_base.cpp
	json_parser_thread.cpp
	parser_base.cpp
	parser_global.cpp
	pstring.cpp
	sax_parser_base.cpp
	sax_token_parser.cpp
	sax_token_parser_thread.cpp
	stream.cpp
	string_pool.cpp
	tokens.cpp
	types.cpp
	xml_namespace.cpp
	yaml_parser_base.cpp
	zip_archive.cpp
	zip_archive_stream.cpp
)

target_compile_definitions(orcus-parser-${ORCUS_API_VERSION} PRIVATE __ORCUS_PSR_BUILDING_DLL)
target_link_libraries(orcus-parser-${ORCUS_API_VERSION} ${ZLIB_LIBRARIES})

# test programs

set(_TESTS
    base64-test
    parser-base-test
    parser-global-test
    sax-parser-test
    sax-token-parser-test
    stream-test
    string-pool-test
    threaded-json-parser-test
    threaded-sax-token-parser-test
    xml-namespace-test
    zip-archive-test
)

foreach(_TEST ${_TESTS})
    set(_TEST_FILE ${_TEST}.cpp)
    string(REPLACE "-" "_" _TEST_FILE ${_TEST_FILE})
    add_executable(${_TEST} EXCLUDE_FROM_ALL ${_TEST_FILE})
    target_link_libraries(${_TEST} orcus-parser-${ORCUS_API_VERSION})
    add_test(${_TEST} ${_TEST})
endforeach()

add_dependencies(check ${_TESTS})

install(
    TARGETS
        orcus-parser-${ORCUS_API_VERSION}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
