add_subdirectory(detail)
add_subdirectory(spreadsheet)

list(APPEND _HEADERS
    base64.hpp
    cell_buffer.hpp
    config.hpp
    css_document_tree.hpp
    css_parser.hpp
    css_parser_base.hpp
    css_selector.hpp
    css_types.hpp
    csv_parser.hpp
    csv_parser_base.hpp
    dom_tree.hpp
    env.hpp
    exception.hpp
    format_detection.hpp
    info.hpp
    interface.hpp
    json_document_tree.hpp
    json_global.hpp
    json_parser.hpp
    json_parser_base.hpp
    json_parser_thread.hpp
    json_structure_tree.hpp
    measurement.hpp
    orcus_csv.hpp
    orcus_gnumeric.hpp
    orcus_import_ods.hpp
    orcus_import_xlsx.hpp
    orcus_json.hpp
    orcus_ods.hpp
    orcus_xls_xml.hpp
    orcus_xlsx.hpp
    orcus_xml.hpp
    parser_base.hpp
    parser_global.hpp
    sax_ns_parser.hpp
    sax_parser.hpp
    sax_parser_base.hpp
    sax_token_parser.hpp
    sax_token_parser_thread.hpp
    stream.hpp
    string_pool.hpp
    threaded_json_parser.hpp
    threaded_sax_token_parser.hpp
    tokens.hpp
    types.hpp
    xml_namespace.hpp
    xml_structure_tree.hpp
    xml_writer.hpp
    yaml_document_tree.hpp
    yaml_parser.hpp
    yaml_parser_base.hpp
    zip_archive.hpp
    zip_archive_stream.hpp
)

if(ORCUS_WITH_PARQUET)
    list(APPEND _HEADERS orcus_parquet.hpp)
endif()

install(FILES ${_HEADERS}
    DESTINATION include/orcus-${ORCUS_API_VERSION}/orcus
)
