
add_library(liborcus SHARED
# core
	config.cpp
	css_document_tree.cpp
	css_selector.cpp
	detection_result.cpp
	dom_tree.cpp
	format_detection.cpp
	formula_result.cpp
	global.cpp
	info.cpp
	interface.cpp
	json_document_tree.cpp
	json_util.cpp
	spreadsheet_interface.cpp
	orcus_csv.cpp
    orcus_json.cpp
	orcus_xml.cpp
	measurement.cpp
	xml_context_base.cpp
	xml_context_global.cpp
	xml_map_tree.cpp
	xml_stream_handler.cpp
	xml_stream_parser.hpp
	xml_stream_parser.cpp
	xml_simple_stream_handler.cpp
	xml_structure_tree.cpp
	yaml_document_tree.cpp
	ooxml_namespace_types.cpp
	ooxml_namespace_types.hpp
	odf_namespace_types.hpp
	odf_namespace_types.cpp
	gnumeric_namespace_types.cpp
	xls_xml_namespace_types.cpp
	session_context.cpp
    spreadsheet_interface.cpp
	spreadsheet_iface_util.cpp
	spreadsheet_types.cpp
    spreadsheet_impl_types.cpp
	string_helper.cpp
# xlsx filter
	ooxml_content_types.cpp
	ooxml_global.cpp
	ooxml_schemas.cpp
	ooxml_tokens.cpp
	ooxml_types.cpp
	opc_context.cpp
	opc_reader.cpp
	orcus_xlsx.cpp
	orcus_import_xlsx.cpp
	xlsx_context.cpp
	xlsx_drawing_context.cpp
	xlsx_handler.cpp
	xlsx_helper.cpp
	xlsx_session_data.cpp
	xlsx_revision_context.cpp
	xlsx_pivot_context.cpp
	xlsx_sheet_context.cpp
	xlsx_conditional_format_context.cpp
	xlsx_table_context.cpp
	xlsx_autofilter_context.cpp
	xlsx_types.cpp
	xlsx_workbook_context.cpp
# xls-xml filter
	xls_xml_tokens.cpp
	orcus_xls_xml.cpp
	xls_xml_detection_handler.cpp
	xls_xml_handler.cpp
	xls_xml_context.cpp
# ods filter
	odf_para_context.cpp
	odf_styles.cpp
	odf_styles_context.cpp
	odf_number_formatting_context.cpp
	odf_tokens.cpp
	ods_content_xml_context.cpp
	ods_content_xml_handler.cpp
	ods_session_data.cpp
	odf_helper.cpp
	orcus_ods.cpp
	orcus_import_ods.cpp
# gnumeric filter
# TODO : document a way to build boost with bzip2 and zlib options, or find
# a way to make this a build-time option.
#   gnumeric_detection_handler.cpp
#   gnumeric_cell_context.cpp
#   gnumeric_context.cpp
#   gnumeric_handler.cpp
#   gnumeric_helper.cpp
#   gnumeric_sheet_context.cpp
#   gnumeric_tokens.cpp
#   orcus_gnumeric.cpp
)

configure_file(constants.inl.in constants.inl)

target_link_libraries(liborcus liborcus-parser)

target_compile_definitions(liborcus PRIVATE
    __ORCUS_BUILDING_DLL
    __ORCUS_XLSX
    __ORCUS_ODS
    __ORCUS_XLS_XML
)

# single-file test programs

set(_TESTS
    common-test
    css-document-tree-test
    format-detection-test
#   gnumeric-cell-context-test
#   gnumeric-helper-test
#   gnumeric-sheet-context-test
    json-document-tree-test
    json-structure-tree-test
#   odf-styles-context-test
    xml-structure-tree-test
    yaml-document-tree-test
)

foreach(_TEST ${_TESTS})
    set(_TEST_FILE ${_TEST}.cpp)
    string(REPLACE "-" "_" _TEST_FILE ${_TEST_FILE})
    add_executable(${_TEST} EXCLUDE_FROM_ALL ${_TEST_FILE})
    target_link_libraries(${_TEST} liborcus liborcus-parser)
    target_compile_definitions(${_TEST} PRIVATE
        SRCDIR="${PROJECT_SOURCE_DIR}"
    )
    add_test(${_TEST} ${_TEST})

    # Make sure the liborcus-parser library file is present in the same directory
    # as the test program.
    add_custom_command(TARGET ${_TEST} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:liborcus-parser>
        $<TARGET_FILE_DIR:${_TEST}>
    )

endforeach()

# multi-file test programs

add_executable(odf-helper-test EXCLUDE_FROM_ALL
    odf_helper_test.cpp
    odf_helper.cpp
    string_helper.cpp
)

add_executable(xlsx-sheet-context-test EXCLUDE_FROM_ALL
    formula_result.cpp
    global.cpp
    mock_spreadsheet.hpp
    mock_spreadsheet.cpp
    ooxml_global.cpp
    ooxml_namespace_types.cpp
    ooxml_schemas.cpp
    ooxml_tokens.cpp
    ooxml_types.cpp
    session_context.cpp
    spreadsheet_interface.cpp
    xlsx_autofilter_context.cpp
    xlsx_conditional_format_context.cpp
    xlsx_helper.cpp
    xlsx_session_data.cpp
    xlsx_sheet_context.cpp
    xlsx_sheet_context_test.cpp
    xlsx_types.cpp
    xml_context_base.cpp
    xml_context_global.cpp
)

add_executable(xml-map-tree-test EXCLUDE_FROM_ALL
    xml_map_tree_test.cpp
    spreadsheet_impl_types.cpp
    xml_map_tree.cpp
)

target_compile_definitions(xlsx-sheet-context-test PRIVATE
    __ORCUS_STATIC_LIB
)

target_link_libraries(odf-helper-test liborcus liborcus-parser)
target_link_libraries(xlsx-sheet-context-test liborcus liborcus-parser)
target_link_libraries(xml-map-tree-test liborcus liborcus-parser)
add_test(odf-helper-test odf-helper-test)
add_test(xlsx-sheet-context-test xlsx-sheet-context-test)
add_test(xml-map-tree-test xml-map-tree-test)

add_dependencies(check
    ${_TESTS}
    odf-helper-test
    xlsx-sheet-context-test
    xml-map-tree-test
)
