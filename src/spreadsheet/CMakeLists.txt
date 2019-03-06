
add_library(liborcus-spreadsheet-model SHARED
    auto_filter.cpp
	config.cpp
	csv_dumper.cpp
	document.cpp
	dumper_global.cpp
	factory.cpp
	factory_pivot.cpp
	factory_sheet.cpp
	factory_styles.cpp
	factory_table.cpp
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
	sheet_range.cpp
	styles.cpp
	view.cpp
)

target_link_libraries(liborcus-spreadsheet-model liborcus-parser liborcus ${IXION_LIB})
target_compile_definitions(liborcus-spreadsheet-model PRIVATE __ORCUS_SPM_BUILDING_DLL)
