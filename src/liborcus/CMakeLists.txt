
set(LIBORCUS_SRC_DIR ${PROJECT_SOURCE_DIR}/src/liborcus)

add_library(liborcus SHARED
# core
	${LIBORCUS_SRC_DIR}/config.cpp
	${LIBORCUS_SRC_DIR}/css_document_tree.cpp
	${LIBORCUS_SRC_DIR}/css_selector.cpp
	${LIBORCUS_SRC_DIR}/detection_result.cpp
	${LIBORCUS_SRC_DIR}/dom_tree.cpp
	${LIBORCUS_SRC_DIR}/format_detection.cpp
	${LIBORCUS_SRC_DIR}/formula_result.cpp
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
	${LIBORCUS_SRC_DIR}/spreadsheet_iface_util.cpp
	${LIBORCUS_SRC_DIR}/spreadsheet_types.cpp
	${LIBORCUS_SRC_DIR}/string_helper.cpp
# xlsx filter
	${LIBORCUS_SRC_DIR}/ooxml_content_types.cpp
	${LIBORCUS_SRC_DIR}/ooxml_global.cpp
	${LIBORCUS_SRC_DIR}/ooxml_schemas.cpp
	${LIBORCUS_SRC_DIR}/ooxml_tokens.cpp
	${LIBORCUS_SRC_DIR}/ooxml_types.cpp
	${LIBORCUS_SRC_DIR}/opc_context.cpp
	${LIBORCUS_SRC_DIR}/opc_reader.cpp
	${LIBORCUS_SRC_DIR}/orcus_xlsx.cpp
	${LIBORCUS_SRC_DIR}/orcus_import_xlsx.cpp
	${LIBORCUS_SRC_DIR}/xlsx_context.cpp
	${LIBORCUS_SRC_DIR}/xlsx_drawing_context.cpp
	${LIBORCUS_SRC_DIR}/xlsx_handler.cpp
	${LIBORCUS_SRC_DIR}/xlsx_helper.cpp
	${LIBORCUS_SRC_DIR}/xlsx_session_data.cpp
	${LIBORCUS_SRC_DIR}/xlsx_revision_context.cpp
	${LIBORCUS_SRC_DIR}/xlsx_pivot_context.cpp
	${LIBORCUS_SRC_DIR}/xlsx_sheet_context.cpp
	${LIBORCUS_SRC_DIR}/xlsx_conditional_format_context.cpp
	${LIBORCUS_SRC_DIR}/xlsx_table_context.cpp
	${LIBORCUS_SRC_DIR}/xlsx_autofilter_context.cpp
	${LIBORCUS_SRC_DIR}/xlsx_types.cpp
	${LIBORCUS_SRC_DIR}/xlsx_workbook_context.cpp
# xls-xml filter
	${LIBORCUS_SRC_DIR}/xls_xml_tokens.cpp
	${LIBORCUS_SRC_DIR}/orcus_xls_xml.cpp
	${LIBORCUS_SRC_DIR}/xls_xml_detection_handler.cpp
	${LIBORCUS_SRC_DIR}/xls_xml_handler.cpp
	${LIBORCUS_SRC_DIR}/xls_xml_context.cpp
# ods filter
	${LIBORCUS_SRC_DIR}/odf_para_context.cpp
	${LIBORCUS_SRC_DIR}/odf_styles.cpp
	${LIBORCUS_SRC_DIR}/odf_styles_context.cpp
	${LIBORCUS_SRC_DIR}/odf_number_formatting_context.cpp
	${LIBORCUS_SRC_DIR}/odf_tokens.cpp
	${LIBORCUS_SRC_DIR}/ods_content_xml_context.cpp
	${LIBORCUS_SRC_DIR}/ods_content_xml_handler.cpp
	${LIBORCUS_SRC_DIR}/ods_session_data.cpp
	${LIBORCUS_SRC_DIR}/odf_helper.cpp
	${LIBORCUS_SRC_DIR}/orcus_ods.cpp
	${LIBORCUS_SRC_DIR}/orcus_import_ods.cpp
# gnumeric filter
# TODO : document a way to build boost with bzip2 and zlib options, or find
# a way to make this a build-time option.
#   ${LIBORCUS_SRC_DIR}/gnumeric_detection_handler.cpp
#   ${LIBORCUS_SRC_DIR}/gnumeric_cell_context.cpp
#   ${LIBORCUS_SRC_DIR}/gnumeric_context.cpp
#   ${LIBORCUS_SRC_DIR}/gnumeric_handler.cpp
#   ${LIBORCUS_SRC_DIR}/gnumeric_helper.cpp
#   ${LIBORCUS_SRC_DIR}/gnumeric_sheet_context.cpp
#   ${LIBORCUS_SRC_DIR}/gnumeric_tokens.cpp
#   ${LIBORCUS_SRC_DIR}/orcus_gnumeric.cpp
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

target_link_libraries(liborcus liborcus-parser)

target_compile_definitions(liborcus PRIVATE
    __ORCUS_BUILDING_DLL
    __ORCUS_XLSX
    __ORCUS_ODS
    __ORCUS_XLS_XML
)
