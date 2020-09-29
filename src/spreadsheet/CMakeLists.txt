
add_library(orcus-spreadsheet-model-${ORCUS_API_VERSION} SHARED
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
	styles.cpp
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
