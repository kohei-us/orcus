
AM_CPPFLAGS = \
	-I$(top_srcdir)/include \
	$(LIBIXION_CFLAGS) \
	-DSRCDIR='"$(top_srcdir)"' \
	-DINPUTDIR='"$(srcdir)/files"'

EXTRA_DIST = \
	files/cities.xml

EXTRA_PROGRAMS = \
	xml-namespace \
	xml-mapping-basic \
	xml-mapping-repeat

SPDOC_LDADD = \
	../../src/parser/liborcus-parser-@ORCUS_API_VERSION@.la \
	../../src/liborcus/liborcus-@ORCUS_API_VERSION@.la \
	../../src/spreadsheet/liborcus-spreadsheet-model-@ORCUS_API_VERSION@.la \
	$(LIBIXION_LIBS)

xml_namespace_SOURCES = \
	xml_namespace.cpp

xml_namespace_LDADD = \
	../../src/parser/liborcus-parser-@ORCUS_API_VERSION@.la \
	../../src/liborcus/liborcus-@ORCUS_API_VERSION@.la

xml_mapping_basic_SOURCES = xml_mapping_basic.cpp
xml_mapping_basic_LDADD = $(SPDOC_LDADD)

xml_mapping_repeat_SOURCES = xml_mapping_repeat.cpp
xml_mapping_repeat_LDADD = $(SPDOC_LDADD)

TESTS = $(EXTRA_PROGRAMS)

distclean-local:
	rm -rf $(TESTS)
