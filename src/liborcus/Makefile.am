
AM_CPPFLAGS = \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/src/include \
	-I./include $(BOOST_CPPFLAGS)

EXTRA_PROGRAMS = \
	css-document-tree-test \
	json-document-tree-test \
	json-path-test \
	yaml-document-tree-test \
	xml-map-tree-test \
	common-test \
	dom-tree-test \
	json-structure-tree-test \
	json-map-tree-test \
	xml-structure-tree-test \
	xpath-parser-test \
	xls-filter-utils-test

TESTS =

EXTRA_DIST = \
	xml_element_types.hpp

if HAVE_STATIC_LIB
AM_CPPFLAGS += -D__ORCUS_STATIC_LIB=1
else
AM_CPPFLAGS += -D__ORCUS_BUILDING_DLL=1
endif

if HAVE_FILESYSTEM
AM_CPPFLAGS += "-DHAVE_FILESYSTEM=1"
endif

if HAVE_EXPERIMENTAL_FILESYSTEM
AM_CPPFLAGS += "-DHAVE_EXPERIMENTAL_FILESYSTEM=1"
endif

liborcus_@ORCUS_API_VERSION@_la_CXXFLAGS = \
	$(ZLIB_CFLAGS)

liborcus_@ORCUS_API_VERSION@_la_LDFLAGS = \
	-no-undefined $(BOOST_SYSTEM_LDFLAGS)

liborcus_@ORCUS_API_VERSION@_la_LIBADD = \
	../parser/liborcus-parser-@ORCUS_API_VERSION@.la \
	 $(BOOST_SYSTEM_LIBS) $(ZLIB_LIBS)

if !HAVE_FILESYSTEM
if HAVE_EXPERIMENTAL_FILESYSTEM
liborcus_@ORCUS_API_VERSION@_la_LDFLAGS += -lstdc++fs
else
liborcus_@ORCUS_API_VERSION@_la_LDFLAGS += $(BOOST_FILESYSTEM_LDFLAGS)
liborcus_@ORCUS_API_VERSION@_la_LIBADD += $(BOOST_FILESYSTEM_LIBS)
endif
endif

AM_CPPFLAGS += -DSRCDIR=\""$(top_srcdir)"\"

lib_LTLIBRARIES = liborcus-@ORCUS_API_VERSION@.la
liborcus_@ORCUS_API_VERSION@_la_SOURCES = \
	config.cpp \
	css_document_tree.cpp \
	css_selector.cpp \
	debug_utils.hpp \
	debug_utils.cpp \
	detection_result.hpp \
	detection_result.cpp \
	dom_tree.cpp \
	format_detection.cpp \
	formula_result.hpp \
	formula_result.cpp \
	impl_utils.hpp \
	info.cpp \
	interface.cpp \
	json_document_tree.cpp \
	json_map_tree.hpp \
	json_map_tree.cpp \
	json_path.hpp \
	json_path.cpp \
	json_structure_mapper.hpp \
	json_structure_mapper.cpp \
	json_structure_tree.cpp \
	json_util.hpp \
	json_util.cpp \
	spreadsheet_interface.cpp \
	orcus_csv.cpp \
	orcus_json.cpp \
	orcus_xml.cpp \
	orcus_xml_impl.hpp \
	orcus_xml_impl.cpp \
	orcus_xml_map_def.cpp \
	measurement.cpp \
	number_utils.cpp \
	number_utils.hpp \
	xml_context_base.hpp \
	xml_context_base.cpp \
	xml_context_global.hpp \
	xml_context_global.cpp \
	xml_element_types.cpp \
	xml_element_validator.hpp \
	xml_element_validator.cpp \
	xml_empty_context.hpp \
	xml_empty_context.cpp \
	xml_map_tree.hpp \
	xml_map_tree.cpp \
	xml_stream_handler.hpp \
	xml_stream_handler.cpp \
	xml_stream_parser.hpp \
	xml_stream_parser.cpp \
	xml_simple_stream_handler.hpp \
	xml_simple_stream_handler.cpp \
	xml_structure_mapper.hpp \
	xml_structure_mapper.cpp \
	xml_structure_tree.cpp \
	xml_util.hpp \
	xml_util.cpp \
	xpath_parser.cpp \
	yaml_document_tree.cpp \
	ooxml_namespace_types.cpp \
	ooxml_namespace_types.hpp \
	odf_namespace_types.hpp \
	odf_namespace_types_hpp.inl \
	odf_namespace_types.cpp \
	odf_namespace_types_cpp.inl \
	gnumeric_namespace_types.hpp \
	gnumeric_namespace_types.cpp \
	xls_xml_namespace_types.hpp \
	xls_xml_namespace_types.cpp \
	xls_types.hpp \
	xls_types.cpp \
	xls_filter_utils.hpp \
	xls_filter_utils.cpp \
	session_context.hpp \
	session_context.cpp \
	spreadsheet_impl_types.hpp \
	spreadsheet_impl_types.cpp \
	spreadsheet_types.cpp \
	spreadsheet_iface_util.hpp \
	spreadsheet_iface_util.cpp \
	string_helper.hpp \
	string_helper.cpp

if WITH_XLSX_FILTER

EXTRA_PROGRAMS += \
	xlsx-sheet-context-test

liborcus_@ORCUS_API_VERSION@_la_SOURCES += \
	ooxml_content_types.cpp \
	ooxml_content_types.hpp \
	ooxml_global.cpp \
	ooxml_global.hpp \
	ooxml_schemas.cpp \
	ooxml_schemas.hpp \
	ooxml_token_constants.hpp \
	ooxml_token_constants.inl \
	ooxml_tokens.cpp \
	ooxml_tokens.hpp \
	ooxml_tokens.inl \
	ooxml_types.hpp \
	ooxml_types.cpp \
	opc_context.cpp \
	opc_context.hpp \
	opc_reader.cpp \
	opc_reader.hpp \
	opc_reader.hpp \
	opc_token_constants.hpp \
	opc_token_constants.inl \
	opc_tokens.inl \
	orcus_xlsx.cpp \
	orcus_import_xlsx.cpp \
	xlsx_shared_strings_context.cpp \
	xlsx_shared_strings_context.hpp \
	xlsx_drawing_context.hpp \
	xlsx_drawing_context.cpp \
	xlsx_handler.cpp \
	xlsx_handler.hpp \
	xlsx_helper.cpp \
	xlsx_helper.hpp \
	xlsx_session_data.hpp \
	xlsx_session_data.cpp \
	xlsx_revision_context.cpp \
	xlsx_revision_context.hpp \
	xlsx_pivot_context.cpp \
	xlsx_pivot_context.hpp \
	xlsx_sheet_context.cpp \
	xlsx_sheet_context.hpp \
	xlsx_styles_context.cpp \
	xlsx_styles_context.hpp \
	xlsx_conditional_format_context.cpp \
	xlsx_conditional_format_context.hpp \
	xlsx_table_context.cpp \
	xlsx_table_context.hpp \
	xlsx_autofilter_context.cpp \
	xlsx_autofilter_context.hpp \
	xlsx_types.hpp \
	xlsx_types.cpp \
	xlsx_workbook_context.cpp \
	xlsx_workbook_context.hpp

# xlsx-sheet-context-test

xlsx_sheet_context_test_SOURCES = \
	debug_utils.cpp \
	formula_result.cpp \
	ooxml_global.cpp \
	ooxml_namespace_types.cpp \
	ooxml_schemas.cpp \
	ooxml_tokens.cpp \
	ooxml_types.cpp \
	session_context.cpp \
	spreadsheet_interface.cpp \
	xlsx_autofilter_context.cpp \
	xlsx_conditional_format_context.cpp \
	xlsx_helper.cpp \
	xlsx_session_data.cpp \
	xlsx_sheet_context.cpp \
	xlsx_sheet_context_test.cpp \
	xlsx_types.cpp \
	xls_filter_utils.cpp \
	xml_context_base.cpp \
	xml_context_global.cpp \
	xml_element_types.cpp \
	xml_element_validator.cpp \
	xml_empty_context.cpp \
	xml_util.cpp

xlsx_sheet_context_test_LDADD = \
	liborcus-@ORCUS_API_VERSION@.la \
	../parser/liborcus-parser-@ORCUS_API_VERSION@.la \
	../test/liborcus-test.a

xlsx_sheet_context_test_CPPFLAGS = -I$(top_builddir)/lib/liborcus/liborcus.la $(AM_CPPFLAGS)

TESTS += \
	 xlsx-sheet-context-test

endif # WITH_XLSX_FILTER

if WITH_XLS_XML_FILTER

liborcus_@ORCUS_API_VERSION@_la_SOURCES += \
	xls_xml_tokens.hpp \
	xls_xml_tokens.inl \
	xls_xml_tokens.cpp \
	xls_xml_token_constants.hpp \
	xls_xml_token_constants.inl \
	orcus_xls_xml.cpp \
	xls_xml_detection_handler.hpp \
	xls_xml_detection_handler.cpp \
	xls_xml_handler.hpp \
	xls_xml_handler.cpp \
	xls_xml_auto_filter_context.hpp \
	xls_xml_auto_filter_context.cpp \
	xls_xml_context.hpp \
	xls_xml_context.cpp

endif # WITH_XLS_XML_FILTER

if WITH_ODS_FILTER

liborcus_@ORCUS_API_VERSION@_la_SOURCES += \
	odf_document_styles_context.hpp \
	odf_document_styles_context.cpp \
	odf_para_context.hpp \
	odf_para_context.cpp \
	odf_styles.hpp \
	odf_styles.cpp \
	odf_styles_context.hpp \
	odf_styles_context.cpp \
	odf_style_context.hpp \
	odf_style_context.cpp \
	odf_number_format_context.hpp \
	odf_number_format_context.cpp \
	odf_token_constants.hpp \
	odf_token_constants.inl \
	odf_tokens.hpp \
	odf_tokens.inl \
	odf_tokens.cpp \
	ods_content_xml_context.hpp \
	ods_content_xml_context.cpp \
	ods_database_ranges_context.hpp \
	ods_database_ranges_context.cpp \
	ods_dde_links_context.hpp \
	ods_dde_links_context.cpp \
	ods_session_data.hpp \
	ods_session_data.cpp \
	odf_helper.hpp \
	odf_helper.cpp \
	orcus_ods.cpp \
	orcus_import_ods.cpp

# odf-helper-test

EXTRA_PROGRAMS += \
	odf-helper-test

odf_helper_test_SOURCES = \
	odf_helper.cpp \
	string_helper.cpp \
	odf_helper_test.cpp

odf_helper_test_LDADD = \
	liborcus-@ORCUS_API_VERSION@.la \
	../parser/liborcus-parser-@ORCUS_API_VERSION@.la

odf_helper_test_CPPFLAGS = -I$(top_builddir)/lib/liborcus/liborcus.la $(AM_CPPFLAGS)

TESTS += \
	 odf-helper-test

endif # WITH_ODS_FILTER

if WITH_GNUMERIC_FILTER

liborcus_@ORCUS_API_VERSION@_la_SOURCES += \
	gnumeric_cell_context.cpp \
	gnumeric_cell_context.hpp \
	gnumeric_context.cpp \
	gnumeric_context.hpp \
	gnumeric_detection_handler.cpp \
	gnumeric_detection_handler.hpp \
	gnumeric_filter_context.cpp \
	gnumeric_filter_context.hpp \
	gnumeric_handler.cpp \
	gnumeric_handler.hpp \
	gnumeric_names_context.cpp \
	gnumeric_names_context.hpp \
	gnumeric_sheet_context.cpp \
	gnumeric_sheet_context.hpp \
	gnumeric_styles_context.cpp \
	gnumeric_styles_context.hpp \
	gnumeric_token_constants.hpp \
	gnumeric_token_constants.inl \
	gnumeric_tokens.cpp \
	gnumeric_tokens.hpp \
	gnumeric_tokens.inl \
	gnumeric_types.cpp \
	gnumeric_types.hpp \
	gnumeric_value_format_parser.cpp \
	gnumeric_value_format_parser.hpp \
	orcus_gnumeric.cpp

liborcus_@ORCUS_API_VERSION@_la_LDFLAGS += \
	$(BOOST_IOSTREAMS_LDFLAGS)

liborcus_@ORCUS_API_VERSION@_la_LIBADD += \
	$(BOOST_IOSTREAMS_LIBS)

# gnumeric-cell-context-test

EXTRA_PROGRAMS += \
	gnumeric-cell-context-test \
	gnumeric-sheet-context-test

gnumeric_cell_context_test_SOURCES = \
	debug_utils.cpp \
	number_utils.cpp \
	session_context.cpp \
	gnumeric_cell_context_test.cpp \
	gnumeric_cell_context.cpp \
	gnumeric_value_format_parser.cpp \
	xml_context_base.cpp \
	xml_element_types.cpp \
	xml_element_validator.cpp \
	xml_empty_context.cpp \
	xml_util.cpp \
	gnumeric_namespace_types.cpp \
	gnumeric_tokens.cpp \
	gnumeric_types.cpp \
	odf_namespace_types.cpp \
	spreadsheet_interface.cpp

gnumeric_cell_context_test_LDADD = \
	liborcus-@ORCUS_API_VERSION@.la \
	../parser/liborcus-parser-@ORCUS_API_VERSION@.la \
	../test/liborcus-test.a

orcus_gnumeric_cell_context_test_CPPFLAGS = -I$(top_builddir)/lib/liborcus/liborcus.la $(AM_CPPFLAGS)

TESTS += gnumeric-cell-context-test

# gnumeric-sheet-context-test

gnumeric_sheet_context_test_SOURCES = \
	debug_utils.cpp \
	session_context.cpp \
	gnumeric_sheet_context_test.cpp \
	gnumeric_sheet_context.cpp \
	gnumeric_names_context.cpp \
	gnumeric_cell_context.cpp \
	gnumeric_filter_context.cpp \
	gnumeric_styles_context.cpp \
	gnumeric_value_format_parser.cpp \
	gnumeric_types.cpp \
	number_utils.cpp \
	xls_filter_utils.cpp \
	xml_context_base.cpp \
	xml_element_types.cpp \
	xml_element_validator.cpp \
	xml_empty_context.cpp \
	xml_util.cpp \
	gnumeric_namespace_types.cpp \
	gnumeric_tokens.cpp \
	odf_namespace_types.cpp \
	spreadsheet_interface.cpp \
	string_helper.cpp

gnumeric_sheet_context_test_LDADD = \
	liborcus-@ORCUS_API_VERSION@.la \
	../parser/liborcus-parser-@ORCUS_API_VERSION@.la \
	../test/liborcus-test.a

gnumeric_sheet_context_test_CPPFLAGS = -I$(top_builddir)/lib/liborcus/liborcus.la $(AM_CPPFLAGS)

TESTS += gnumeric-sheet-context-test

endif # WITH_GNUMERIC_FILTER

if WITH_PARQUET_FILTER

liborcus_@ORCUS_API_VERSION@_la_SOURCES += \
	orcus_parquet.cpp

liborcus_@ORCUS_API_VERSION@_la_CXXFLAGS += \
	$(PARQUET_CFLAGS)

liborcus_@ORCUS_API_VERSION@_la_LDFLAGS += \
	$(PARQUET_LDFLAGS)

liborcus_@ORCUS_API_VERSION@_la_LIBADD += \
	$(PARQUET_LIBS)

endif # WITH_PARQUET_FILTER

# css-document-tree-test

css_document_tree_test_SOURCES = \
	css_document_tree.cpp \
	css_document_tree_test.cpp

css_document_tree_test_LDADD = \
	liborcus-@ORCUS_API_VERSION@.la \
	../parser/liborcus-parser-@ORCUS_API_VERSION@.la \
	$(BOOST_SYSTEM_LIBS)

if !HAVE_FILESYSTEM
if HAVE_EXPERIMENTAL_FILESYSTEM
css_document_tree_test_LDADD += -lstdc++fs
else
css_document_tree_test_LDADD += $(BOOST_FILESYSTEM_LIBS)
endif
endif

# json-document-tree-test

json_document_tree_test_SOURCES = \
	json_document_tree.cpp \
	json_path.cpp \
	json_util.cpp \
	json_document_tree_test.cpp

json_document_tree_test_LDADD = \
	liborcus-@ORCUS_API_VERSION@.la \
	../parser/liborcus-parser-@ORCUS_API_VERSION@.la \
	../test/liborcus-test.a \
	$(BOOST_SYSTEM_LIBS)

if !HAVE_FILESYSTEM
if HAVE_EXPERIMENTAL_FILESYSTEM
json_document_tree_test_LDADD += -lstdc++fs
else
json_document_tree_test_LDADD += $(BOOST_FILESYSTEM_LIBS)
endif
endif

json_document_tree_test_CPPFLAGS = -I$(top_builddir)/lib/liborcus/liborcus.la $(AM_CPPFLAGS)

# json-path-test

json_path_test_SOURCES = \
	json_path.cpp \
	json_path_test.cpp

json_path_test_LDADD = \
	liborcus-@ORCUS_API_VERSION@.la \
	../test/liborcus-test.a \
	../parser/liborcus-parser-@ORCUS_API_VERSION@.la

# yaml-document-tree-test

yaml_document_tree_test_SOURCES = \
	yaml_document_tree.cpp \
	json_util.cpp \
	yaml_document_tree_test.cpp

yaml_document_tree_test_LDADD = \
	liborcus-@ORCUS_API_VERSION@.la \
	../parser/liborcus-parser-@ORCUS_API_VERSION@.la \
	$(BOOST_SYSTEM_LIBS)

if !HAVE_FILESYSTEM
if HAVE_EXPERIMENTAL_FILESYSTEM
yaml_document_tree_test_LDADD += -lstdc++fs
else
yaml_document_tree_test_LDADD += $(BOOST_FILESYSTEM_LIBS)
endif
endif

yaml_document_tree_test_CPPFLAGS = -I$(top_builddir)/lib/liborcus/liborcus.la $(AM_CPPFLAGS)

# xml-map-tree-test

xml_map_tree_test_SOURCES = \
	xml_map_tree.cpp \
	xml_map_tree.hpp \
	xpath_parser.hpp \
	xpath_parser.cpp \
	spreadsheet_impl_types.hpp \
	spreadsheet_impl_types.cpp \
	xml_map_tree_test.cpp

xml_map_tree_test_LDADD = \
	liborcus-@ORCUS_API_VERSION@.la \
	../test/liborcus-test.a \
	../parser/liborcus-parser-@ORCUS_API_VERSION@.la \
    $(BOOST_SYSTEM_LIBS)

# xml-structure-tree-test

xml_structure_tree_test_SOURCES = \
	string_helper.cpp \
	xml_structure_tree.cpp \
	xml_structure_mapper.cpp \
	xml_structure_tree_test.cpp

xml_structure_tree_test_CPPFLAGS = -I$(top_builddir)/lib/liborcus/liborcus.la $(AM_CPPFLAGS)

xml_structure_tree_test_LDADD = \
	liborcus-@ORCUS_API_VERSION@.la \
	../parser/liborcus-parser-@ORCUS_API_VERSION@.la \
	$(BOOST_SYSTEM_LIBS)

if !HAVE_FILESYSTEM
if HAVE_EXPERIMENTAL_FILESYSTEM
xml_structure_tree_test_LDADD += -lstdc++fs
else
xml_structure_tree_test_LDADD += $(BOOST_FILESYSTEM_LIBS)
endif
endif

# common-test

common_test_SOURCES = \
	common_test.cpp

common_test_LDADD = \
	liborcus-@ORCUS_API_VERSION@.la \
	../parser/liborcus-parser-@ORCUS_API_VERSION@.la

# dom-tree-test

dom_tree_test_SOURCES = dom_tree_test.cpp
dom_tree_test_LDADD = \
	liborcus-@ORCUS_API_VERSION@.la \
	../parser/liborcus-parser-@ORCUS_API_VERSION@.la

# json-structure-tree-test

json_structure_tree_test_SOURCES = json_structure_tree_test.cpp
json_structure_tree_test_LDADD = \
	liborcus-@ORCUS_API_VERSION@.la \
	../parser/liborcus-parser-@ORCUS_API_VERSION@.la \
	$(BOOST_SYSTEM_LIBS)

if !HAVE_FILESYSTEM
if HAVE_EXPERIMENTAL_FILESYSTEM
json_structure_tree_test_LDADD += -lstdc++fs
else
json_structure_tree_test_LDADD += $(BOOST_FILESYSTEM_LIBS)
endif
endif

# json-map-tree-test

json_map_tree_test_SOURCES = json_map_tree_test.cpp \
	json_map_tree.cpp \
	spreadsheet_impl_types.cpp
json_map_tree_test_LDADD = \
	liborcus-@ORCUS_API_VERSION@.la \
	../parser/liborcus-parser-@ORCUS_API_VERSION@.la \
	$(BOOST_SYSTEM_LIBS)

# xpath-parser-test

xpath_parser_test_SOURCES = \
	xpath_parser_test.cpp \
	xpath_parser.cpp
xpath_parser_test_LDADD = \
	liborcus-@ORCUS_API_VERSION@.la \
	../parser/liborcus-parser-@ORCUS_API_VERSION@.la \
	$(BOOST_SYSTEM_LIBS)

# xls-filter-utils-test

xls_filter_utils_test_SOURCES = \
	xls_filter_utils_test.cpp \
	xls_filter_utils.cpp
xls_filter_utils_test_LDADD = \
	liborcus-@ORCUS_API_VERSION@.la \
	../test/liborcus-test.a \
	../parser/liborcus-parser-@ORCUS_API_VERSION@.la \
	$(BOOST_SYSTEM_LIBS)

TESTS += \
	css-document-tree-test \
	json-document-tree-test \
	json-path-test \
	yaml-document-tree-test \
	xml-map-tree-test \
	common-test \
	dom-tree-test \
	json-structure-tree-test \
	json-map-tree-test \
	xml-structure-tree-test \
	xpath-parser-test \
	xls-filter-utils-test

distclean-local:
	rm -rf $(TESTS)

@VALGRIND_CHECK_RULES@
