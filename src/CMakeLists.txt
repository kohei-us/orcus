
add_subdirectory(test)
add_subdirectory(parser)
add_subdirectory(liborcus)
add_subdirectory(spreadsheet)

add_executable(orcus-detect
    orcus_detect_main.cpp
)

add_executable(orcus-css-dump
    orcus_css_dump.cpp
)

add_executable(orcus-zip-dump
    orcus_zip_dump.cpp
)

add_executable(orcus-json
    cli_global.cpp
    orcus_json_cli.cpp
    orcus_json_cli_map.cpp
)

add_executable(orcus-yaml
    orcus_yaml_main.cpp
)

add_executable(orcus-csv
    orcus_csv_main.cpp
    orcus_filter_global.cpp
)

add_executable(orcus-xml
    orcus_xml_main.cpp
    cli_global.cpp
    orcus_filter_global.cpp
)

add_executable(orcus-ods
    orcus_ods_main.cpp
    orcus_filter_global.cpp
)

add_executable(orcus-styles-ods
    orcus_ods_styles.cpp
    orcus_filter_global.cpp
)

add_executable(orcus-xlsx
    orcus_xlsx_main.cpp
    orcus_filter_global.cpp
)

add_executable(orcus-xls-xml
    orcus_xls_xml_main.cpp
    orcus_filter_global.cpp
)

target_link_libraries(orcus-detect orcus-${ORCUS_API_VERSION})
target_link_libraries(orcus-css-dump orcus-parser-${ORCUS_API_VERSION} orcus-${ORCUS_API_VERSION})
target_link_libraries(orcus-zip-dump orcus-parser-${ORCUS_API_VERSION})
target_link_libraries(orcus-json orcus-parser-${ORCUS_API_VERSION} orcus-${ORCUS_API_VERSION})
target_link_libraries(orcus-yaml orcus-parser-${ORCUS_API_VERSION} orcus-${ORCUS_API_VERSION})
target_link_libraries(orcus-csv orcus-parser-${ORCUS_API_VERSION} orcus-spreadsheet-model-${ORCUS_API_VERSION} orcus-${ORCUS_API_VERSION})
target_link_libraries(orcus-xml orcus-parser-${ORCUS_API_VERSION} orcus-spreadsheet-model-${ORCUS_API_VERSION} orcus-${ORCUS_API_VERSION})
target_link_libraries(orcus-ods orcus-parser-${ORCUS_API_VERSION} orcus-spreadsheet-model-${ORCUS_API_VERSION} orcus-${ORCUS_API_VERSION})
target_link_libraries(orcus-styles-ods orcus-parser-${ORCUS_API_VERSION} orcus-spreadsheet-model-${ORCUS_API_VERSION} orcus-${ORCUS_API_VERSION})
target_link_libraries(orcus-xlsx orcus-parser-${ORCUS_API_VERSION} orcus-spreadsheet-model-${ORCUS_API_VERSION} orcus-${ORCUS_API_VERSION})
target_link_libraries(orcus-xls-xml orcus-parser-${ORCUS_API_VERSION} orcus-spreadsheet-model-${ORCUS_API_VERSION} orcus-${ORCUS_API_VERSION})

add_executable(orcus-env-dump EXCLUDE_FROM_ALL
    orcus_env_dump.cpp
)

target_link_libraries(orcus-env-dump orcus-parser-${ORCUS_API_VERSION} orcus-${ORCUS_API_VERSION})

# test programs

add_executable(orcus-test-csv EXCLUDE_FROM_ALL
    orcus_test_csv.cpp
    orcus_test_global.cpp
)

add_executable(orcus-test-ods EXCLUDE_FROM_ALL
    orcus_test_ods.cpp
)

add_executable(orcus-test-xls-xml EXCLUDE_FROM_ALL
    orcus_test_xls_xml.cpp
    orcus_test_global.cpp
)

add_executable(orcus-test-xlsx EXCLUDE_FROM_ALL
    orcus_test_xlsx.cpp
)

add_executable(orcus-test-xml EXCLUDE_FROM_ALL
    orcus_test_xml.cpp
)

add_executable(orcus-test-xml-mapped EXCLUDE_FROM_ALL
    orcus_test_xml_mapped.cpp
    orcus_test_global.cpp
)

add_executable(orcus-test-json-mapped EXCLUDE_FROM_ALL
    orcus_test_json_mapped.cpp
)

add_executable(orcus-test-import-ods EXCLUDE_FROM_ALL
    orcus_test_import_ods.cpp
)

list(APPEND _TESTS
    orcus-test-csv
    orcus-test-ods
    orcus-test-xls-xml
    orcus-test-xlsx
    orcus-test-xml
    orcus-test-xml-mapped
    orcus-test-json-mapped
    orcus-test-import-ods
)

list(APPEND _INSTALL_TARGETS
    orcus-detect
    orcus-css-dump
    orcus-zip-dump
    orcus-json
    orcus-yaml
    orcus-csv
    orcus-xml
    orcus-ods
    orcus-styles-ods
    orcus-xlsx
    orcus-xls-xml
)

if(ORCUS_WITH_PARQUET)
    add_executable(orcus-parquet
        orcus_parquet_main.cpp
        orcus_filter_global.cpp
    )
    target_link_libraries(orcus-parquet
        orcus-parser-${ORCUS_API_VERSION}
        orcus-spreadsheet-model-${ORCUS_API_VERSION}
        orcus-${ORCUS_API_VERSION}
    )
    list(APPEND _INSTALL_TARGETS orcus-parquet)

    add_executable(orcus-test-parquet EXCLUDE_FROM_ALL orcus_test_parquet.cpp)
    list(APPEND _TESTS orcus-test-parquet)
endif()

foreach(_TEST ${_TESTS})
    target_link_libraries(${_TEST}
        orcus-parser-${ORCUS_API_VERSION}
        orcus-spreadsheet-model-${ORCUS_API_VERSION}
        orcus-${ORCUS_API_VERSION}
        orcus-test
    )

    target_compile_definitions(${_TEST} PRIVATE
        SRCDIR="${PROJECT_SOURCE_DIR}"
    )

    add_test(${_TEST} ${_TEST})

    # Make sure the liborcus-parser library file is present in the same directory
    # as the test program.
    add_custom_command(TARGET ${_TEST} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:orcus-parser-${ORCUS_API_VERSION}>
        $<TARGET_FILE:orcus-${ORCUS_API_VERSION}>
        $<TARGET_FILE:orcus-spreadsheet-model-${ORCUS_API_VERSION}>
        $<TARGET_FILE_DIR:${_TEST}>
    )
endforeach()

add_dependencies(check ${_TESTS})

install(
    TARGETS ${_INSTALL_TARGETS}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
