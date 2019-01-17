
add_subdirectory(parser)
add_subdirectory(liborcus)
add_subdirectory(spreadsheet)

add_executable(orcus-detect
	${PROJECT_SOURCE_DIR}/src/orcus_detect_main.cpp
)

add_executable(orcus-css-dump
	${PROJECT_SOURCE_DIR}/src/orcus_css_dump.cpp
)

add_executable(orcus-xml-dump
	${PROJECT_SOURCE_DIR}/src/orcus_xml_dump.cpp
	${PROJECT_SOURCE_DIR}/src/dom_tree_sax_handler.cpp
)

add_executable(orcus-zip-dump
	${PROJECT_SOURCE_DIR}/src/orcus_zip_dump.cpp
)

add_executable(orcus-json
	${PROJECT_SOURCE_DIR}/src/orcus_json_main.cpp
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
    ${PROJECT_SOURCE_DIR}/src/xml_map_sax_handler.cpp
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

target_link_libraries(orcus-detect liborcus)
target_link_libraries(orcus-css-dump liborcus-parser liborcus)
target_link_libraries(orcus-xml-dump liborcus-parser liborcus)
target_link_libraries(orcus-zip-dump liborcus-parser)
target_link_libraries(orcus-json liborcus-parser liborcus)
target_link_libraries(orcus-yaml liborcus-parser liborcus)
target_link_libraries(orcus-csv liborcus-parser liborcus-spreadsheet-model liborcus)
target_link_libraries(orcus-xml liborcus-parser liborcus-spreadsheet-model liborcus)
target_link_libraries(orcus-ods liborcus-parser liborcus-spreadsheet-model liborcus)
target_link_libraries(orcus-styles-ods liborcus-parser liborcus-spreadsheet-model liborcus)
target_link_libraries(orcus-xlsx liborcus-parser liborcus-spreadsheet-model liborcus)
target_link_libraries(orcus-xls-xml liborcus-parser liborcus-spreadsheet-model liborcus)
