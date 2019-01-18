
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
	spreadsheet_iface_util.cpp
	spreadsheet_types.cpp
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

add_custom_target(
    py_gen_files
    COMMAND python ${PROJECT_SOURCE_DIR}/bin/gen-files.py
        --properties
            ORCUS_MAJOR_VERSION=${ORCUS_MAJOR_VERSION}
            ORCUS_MINOR_VERSION=${ORCUS_MINOR_VERSION}
            ORCUS_MICRO_VERSION=${ORCUS_MICRO_VERSION}
        --files
            ${CMAKE_CURRENT_SOURCE_DIR}/constants.inl
)

add_dependencies(liborcus py_gen_files)

target_link_libraries(liborcus liborcus-parser)

target_compile_definitions(liborcus PRIVATE
    __ORCUS_BUILDING_DLL
    __ORCUS_XLSX
    __ORCUS_ODS
    __ORCUS_XLS_XML
)
