
AM_CPPFLAGS = \
	-I$(top_srcdir)/include \
	$(LIBIXION_CFLAGS) \
	-DSRCDIR='"$(top_srcdir)"' \
	-DINPUTDIR='"$(srcdir)/files"'

EXTRA_DIST = \
	files/cities.xml \
	files/library.xml \
	files/server-logs.xml

EXTRA_PROGRAMS = \
	sax-parser-1 \
	sax-ns-parser-1 \
	sax-token-parser-1 \
	dom-tree-1 \
	xml-namespace \
	xml-mapping-basic \
	xml-mapping-repeat \
	xml-mapping-ns \
	xml-mapping-autodetect

SPDOC_LDADD = \
	../../src/parser/liborcus-parser-@ORCUS_API_VERSION@.la \
	../../src/liborcus/liborcus-@ORCUS_API_VERSION@.la \
	../../src/spreadsheet/liborcus-spreadsheet-model-@ORCUS_API_VERSION@.la \
	$(LIBIXION_LIBS)

PSR_LDADD = \
	../../src/parser/liborcus-parser-@ORCUS_API_VERSION@.la \
	../../src/liborcus/liborcus-@ORCUS_API_VERSION@.la

sax_parser_1_SOURCES = sax_parser_1.cpp
sax_parser_1_LDADD = $(PSR_LDADD)

sax_ns_parser_1_SOURCES = sax_ns_parser_1.cpp
sax_ns_parser_1_LDADD = $(PSR_LDADD)

sax_token_parser_1_SOURCES = sax_token_parser_1.cpp
sax_token_parser_1_LDADD = $(PSR_LDADD)

dom_tree_1_SOURCES = dom_tree_1.cpp
dom_tree_1_LDADD = $(PSR_LDADD)

xml_namespace_SOURCES = \
	xml_namespace.cpp

xml_namespace_LDADD = $(PSR_LDADD)

xml_mapping_basic_SOURCES = xml_mapping_basic.cpp
xml_mapping_basic_LDADD = $(SPDOC_LDADD)

xml_mapping_repeat_SOURCES = xml_mapping_repeat.cpp
xml_mapping_repeat_LDADD = $(SPDOC_LDADD)

xml_mapping_ns_SOURCES = xml_mapping_ns.cpp
xml_mapping_ns_LDADD = $(SPDOC_LDADD)

xml_mapping_autodetect_SOURCES = xml_mapping_autodetect.cpp
xml_mapping_autodetect_LDADD = $(SPDOC_LDADD)

TESTS = $(EXTRA_PROGRAMS)

distclean-local:
	rm -rf $(TESTS)
