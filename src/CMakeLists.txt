
add_subdirectory(parser)
add_subdirectory(liborcus)
add_subdirectory(spreadsheet)

add_executable(orcus-detect
	${PROJECT_SOURCE_DIR}/src/orcus_detect_main.cpp
)

add_executable(orcus-css-dump
	${PROJECT_SOURCE_DIR}/src/orcus_css_dump.cpp
)

add_executable(orcus-zip-dump
	${PROJECT_SOURCE_DIR}/src/orcus_zip_dump.cpp
)

add_executable(orcus-json
	${PROJECT_SOURCE_DIR}/src/cli_global.cpp
	${PROJECT_SOURCE_DIR}/src/orcus_json_cli.cpp
	${PROJECT_SOURCE_DIR}/src/orcus_json_cli_map.cpp
)

add_executable(orcus-yaml
	${PROJECT_SOURCE_DIR}/src/orcus_yaml_main.cpp
)

add_executable(orcus-csv
    ${PROJECT_SOURCE_DIR}/src/orcus_csv_main.cpp
    ${PROJECT_SOURCE_DIR}/src/orcus_filter_global.cpp
)

add_executable(orcus-xml
    ${PROJECT_SOURCE_DIR}/src/orcus_xml_main.cpp
    ${PROJECT_SOURCE_DIR}/src/cli_global.cpp
    ${PROJECT_SOURCE_DIR}/src/orcus_filter_global.cpp
)

add_executable(orcus-ods
    ${PROJECT_SOURCE_DIR}/src/orcus_ods_main.cpp
    ${PROJECT_SOURCE_DIR}/src/orcus_filter_global.cpp
)

add_executable(orcus-styles-ods
    ${PROJECT_SOURCE_DIR}/src/orcus_ods_styles.cpp
    ${PROJECT_SOURCE_DIR}/src/orcus_filter_global.cpp
)

add_executable(orcus-xlsx
    ${PROJECT_SOURCE_DIR}/src/orcus_xlsx_main.cpp
    ${PROJECT_SOURCE_DIR}/src/orcus_filter_global.cpp
)

add_executable(orcus-xls-xml
    ${PROJECT_SOURCE_DIR}/src/orcus_xls_xml_main.cpp
    ${PROJECT_SOURCE_DIR}/src/orcus_filter_global.cpp
)

target_link_libraries(orcus-detect liborcus-${ORCUS_API_VERSION})
target_link_libraries(orcus-css-dump liborcus-parser-${ORCUS_API_VERSION} liborcus-${ORCUS_API_VERSION})
target_link_libraries(orcus-zip-dump liborcus-parser-${ORCUS_API_VERSION})
target_link_libraries(orcus-json liborcus-parser-${ORCUS_API_VERSION} liborcus-${ORCUS_API_VERSION})
target_link_libraries(orcus-yaml liborcus-parser-${ORCUS_API_VERSION} liborcus-${ORCUS_API_VERSION})
target_link_libraries(orcus-csv liborcus-parser-${ORCUS_API_VERSION} liborcus-spreadsheet-model-${ORCUS_API_VERSION} liborcus-${ORCUS_API_VERSION})
target_link_libraries(orcus-xml liborcus-parser-${ORCUS_API_VERSION} liborcus-spreadsheet-model-${ORCUS_API_VERSION} liborcus-${ORCUS_API_VERSION})
target_link_libraries(orcus-ods liborcus-parser-${ORCUS_API_VERSION} liborcus-spreadsheet-model-${ORCUS_API_VERSION} liborcus-${ORCUS_API_VERSION})
target_link_libraries(orcus-styles-ods liborcus-parser-${ORCUS_API_VERSION} liborcus-spreadsheet-model-${ORCUS_API_VERSION} liborcus-${ORCUS_API_VERSION})
target_link_libraries(orcus-xlsx liborcus-parser-${ORCUS_API_VERSION} liborcus-spreadsheet-model-${ORCUS_API_VERSION} liborcus-${ORCUS_API_VERSION})
target_link_libraries(orcus-xls-xml liborcus-parser-${ORCUS_API_VERSION} liborcus-spreadsheet-model-${ORCUS_API_VERSION} liborcus-${ORCUS_API_VERSION})

add_executable(orcus-env-dump EXCLUDE_FROM_ALL
    orcus_env_dump.cpp
)

target_link_libraries(orcus-env-dump liborcus-parser-${ORCUS_API_VERSION} liborcus-${ORCUS_API_VERSION})

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
)

add_executable(orcus-test-json-mapped EXCLUDE_FROM_ALL
	orcus_test_json_mapped.cpp
)

set(_TESTS
    orcus-test-csv
    orcus-test-ods
    orcus-test-xls-xml
    orcus-test-xlsx
    orcus-test-xml
    orcus-test-xml-mapped
    orcus-test-json-mapped
)

foreach(_TEST ${_TESTS})
    target_link_libraries(${_TEST} liborcus-parser-${ORCUS_API_VERSION} liborcus-spreadsheet-model-${ORCUS_API_VERSION} liborcus-${ORCUS_API_VERSION})

    target_compile_definitions(${_TEST} PRIVATE
        SRCDIR="${PROJECT_SOURCE_DIR}"
    )

    add_test(${_TEST} ${_TEST})

    # Make sure the liborcus-parser library file is present in the same directory
    # as the test program.
    add_custom_command(TARGET ${_TEST} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:liborcus-parser-${ORCUS_API_VERSION}>
        $<TARGET_FILE:liborcus-${ORCUS_API_VERSION}>
        $<TARGET_FILE:liborcus-spreadsheet-model-${ORCUS_API_VERSION}>
        $<TARGET_FILE_DIR:${_TEST}>
    )
endforeach()

add_dependencies(check ${_TESTS})

install(
    TARGETS
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
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
