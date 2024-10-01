
add_library(orcus-spreadsheet-model-${ORCUS_API_VERSION} SHARED
    auto_filter.cpp
    check_dumper.cpp
    config.cpp
    csv_dumper.cpp
    debug_state_dumper.cpp
    document.cpp
    document_impl.cpp
    document_types.cpp
    dumper_global.cpp
    factory.cpp
    factory_auto_filter.cpp
    factory_pivot.cpp
    factory_shared_strings.cpp
    factory_sheet.cpp
    factory_strikethrough.cpp
    factory_styles.cpp
    factory_table.cpp
    factory_underline.cpp
    flat_dumper.cpp
    formula_global.cpp
    global_settings.cpp
    html_dumper.cpp
    json_dumper.cpp
    number_format.cpp
    pivot.cpp
    shared_formula.cpp
    shared_strings.cpp
    sheet.cpp
    sheet_impl.cpp
    styles.cpp
    table.cpp
    tables.cpp
    view.cpp
)

target_link_libraries(orcus-spreadsheet-model-${ORCUS_API_VERSION} orcus-parser-${ORCUS_API_VERSION} orcus-${ORCUS_API_VERSION} ${IXION_LIB})
target_compile_definitions(orcus-spreadsheet-model-${ORCUS_API_VERSION} PRIVATE __ORCUS_SPM_BUILDING_DLL)

install(
    TARGETS
        orcus-spreadsheet-model-${ORCUS_API_VERSION}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
