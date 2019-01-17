
set(SPREADSHEET_SRC_DIR ${PROJECT_SOURCE_DIR}/src/spreadsheet)

add_library(liborcus-spreadsheet-model SHARED
	${SPREADSHEET_SRC_DIR}/auto_filter.cpp
	${SPREADSHEET_SRC_DIR}/config.cpp
	${SPREADSHEET_SRC_DIR}/csv_dumper.cpp
	${SPREADSHEET_SRC_DIR}/document.cpp
	${SPREADSHEET_SRC_DIR}/dumper_global.cpp
	${SPREADSHEET_SRC_DIR}/factory.cpp
	${SPREADSHEET_SRC_DIR}/factory_pivot.cpp
	${SPREADSHEET_SRC_DIR}/factory_sheet.cpp
	${SPREADSHEET_SRC_DIR}/factory_styles.cpp
	${SPREADSHEET_SRC_DIR}/factory_table.cpp
	${SPREADSHEET_SRC_DIR}/flat_dumper.cpp
	${SPREADSHEET_SRC_DIR}/formula_global.cpp
	${SPREADSHEET_SRC_DIR}/global_settings.cpp
	${SPREADSHEET_SRC_DIR}/html_dumper.cpp
	${SPREADSHEET_SRC_DIR}/json_dumper.cpp
	${SPREADSHEET_SRC_DIR}/pivot.cpp
	${SPREADSHEET_SRC_DIR}/shared_formula.cpp
	${SPREADSHEET_SRC_DIR}/shared_strings.cpp
	${SPREADSHEET_SRC_DIR}/sheet.cpp
	${SPREADSHEET_SRC_DIR}/sheet_range.cpp
	${SPREADSHEET_SRC_DIR}/styles.cpp
	${SPREADSHEET_SRC_DIR}/view.cpp
)

target_link_libraries(liborcus-spreadsheet-model liborcus-parser liborcus ${IXION_LIB})
target_compile_definitions(liborcus-spreadsheet-model PRIVATE __ORCUS_SPM_BUILDING_DLL)
