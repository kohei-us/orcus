add_subdirectory(detail)
add_subdirectory(spreadsheet)

file(GLOB _HEADERS *.hpp)

install(FILES ${_HEADERS}
    DESTINATION include/orcus-${ORCUS_API_VERSION}/orcus
)
