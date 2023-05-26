
list(APPEND _HEADERS
    auto_filter.hpp
    config.hpp
    document.hpp
    document_types.hpp
    export_interface.hpp
    factory.hpp
    import_interface.hpp
    import_interface_pivot.hpp
    import_interface_styles.hpp
    import_interface_view.hpp
    pivot.hpp
    shared_strings.hpp
    sheet.hpp
    styles.hpp
    types.hpp
    view.hpp
    view_types.hpp
)

install(FILES ${_HEADERS}
    DESTINATION include/orcus-${ORCUS_API_VERSION}/orcus/spreadsheet
)
