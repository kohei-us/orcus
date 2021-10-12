
AM_CPPFLAGS = \
	-I$(top_srcdir)/include \
	$(LIBIXION_CFLAGS) \
	-DSRCDIR=\""$(top_srcdir)"\"

bin_PROGRAMS =

EXTRA_PROGRAMS = \
	json-doc-1 \
	json-doc-2 \
	json-parser-1 \
	xml-mapping-1 \
	spreadsheet-doc-1 \
	spreadsheet-doc-1-num-and-formula \
	spreadsheet-doc-2 \
	spreadsheet-doc-2-sheets-no-string-pool \
	spreadsheet-doc-2-sheets-with-string-pool \
	spreadsheet-doc-2-sheets-with-formula

EXTRA_DIST = \
	spreadsheet-doc-1.run

json_doc_1_SOURCES = \
	json_doc_1.cpp

json_doc_1_LDADD = \
	../src/liborcus/liborcus-@ORCUS_API_VERSION@.la \
	../src/parser/liborcus-parser-@ORCUS_API_VERSION@.la

json_doc_2_SOURCES = \
	json_doc_2.cpp

json_doc_2_LDADD = \
	../src/liborcus/liborcus-@ORCUS_API_VERSION@.la \
	../src/parser/liborcus-parser-@ORCUS_API_VERSION@.la

json_parser_1_SOURCES = \
	json_parser_1.cpp

json_parser_1_LDADD = \
	../src/parser/liborcus-parser-@ORCUS_API_VERSION@.la

xml_mapping_1_SOURCES = \
	xml_mapping_1.cpp

xml_mapping_1_LDADD = \
	../src/parser/liborcus-parser-@ORCUS_API_VERSION@.la \
	../src/liborcus/liborcus-@ORCUS_API_VERSION@.la

# Spreadsheet document examples.

SPDOC_LDADD = \
	../src/parser/liborcus-parser-@ORCUS_API_VERSION@.la \
	../src/liborcus/liborcus-@ORCUS_API_VERSION@.la \
	../src/spreadsheet/liborcus-spreadsheet-model-@ORCUS_API_VERSION@.la \
	$(LIBIXION_LIBS)

spreadsheet_doc_1_SOURCES = \
	spreadsheet_doc_1.cpp

spreadsheet_doc_1_LDADD = $(SPDOC_LDADD)

spreadsheet_doc_1_num_and_formula_SOURCES = \
	spreadsheet_doc_1_num_and_formula.cpp

spreadsheet_doc_1_num_and_formula_LDADD = $(SPDOC_LDADD)

spreadsheet_doc_2_SOURCES = \
	spreadsheet_doc_2.cpp

spreadsheet_doc_2_LDADD = \
	../src/parser/liborcus-parser-@ORCUS_API_VERSION@.la \
	../src/liborcus/liborcus-@ORCUS_API_VERSION@.la

spreadsheet_doc_2_sheets_no_string_pool_SOURCES = \
	spreadsheet_doc_2_sheets_no_string_pool.cpp

spreadsheet_doc_2_sheets_no_string_pool_LDADD = \
	../src/parser/liborcus-parser-@ORCUS_API_VERSION@.la \
	../src/liborcus/liborcus-@ORCUS_API_VERSION@.la

spreadsheet_doc_2_sheets_with_string_pool_SOURCES = \
	spreadsheet_doc_2_sheets_with_string_pool.cpp

spreadsheet_doc_2_sheets_with_string_pool_LDADD = \
	../src/parser/liborcus-parser-@ORCUS_API_VERSION@.la \
	../src/liborcus/liborcus-@ORCUS_API_VERSION@.la

spreadsheet_doc_2_sheets_with_formula_SOURCES = \
	spreadsheet_doc_2_sheets_with_formula.cpp

spreadsheet_doc_2_sheets_with_formula_LDADD = \
	../src/parser/liborcus-parser-@ORCUS_API_VERSION@.la \
	../src/liborcus/liborcus-@ORCUS_API_VERSION@.la

spreadsheet-doc-1.run: spreadsheet-doc-1

.PHONY: spreadsheet-doc-1.run

TESTS = \
	json-doc-1 \
	json-doc-2 \
	json-parser-1 \
	xml-mapping-1 \
	spreadsheet-doc-1.run \
	spreadsheet-doc-1-num-and-formula \
	spreadsheet-doc-2 \
	spreadsheet-doc-2-sheets-no-string-pool \
	spreadsheet-doc-2-sheets-with-string-pool \
	spreadsheet-doc-2-sheets-with-formula

distclean-local:
	rm -rf $(TESTS)
