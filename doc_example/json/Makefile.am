
AM_CPPFLAGS = \
	-I$(top_srcdir)/include \
	$(LIBIXION_CFLAGS) \
	-DSRCDIR='"$(top_srcdir)"' \
	-DINPUTDIR='"$(srcdir)/files"'

EXTRA_DIST = \
	files/books.json \
	files/planets.json

EXTRA_PROGRAMS = \
	json-mapping-basic \
	json-mapping-range \
	json-mapping-autodetect

SPDOC_LDADD = \
	../../src/parser/liborcus-parser-@ORCUS_API_VERSION@.la \
	../../src/liborcus/liborcus-@ORCUS_API_VERSION@.la \
	../../src/spreadsheet/liborcus-spreadsheet-model-@ORCUS_API_VERSION@.la \
	$(LIBIXION_LIBS)

json_mapping_basic_SOURCES = json_mapping_basic.cpp
json_mapping_basic_LDADD = $(SPDOC_LDADD)

json_mapping_range_SOURCES = json_mapping_range.cpp
json_mapping_range_LDADD = $(SPDOC_LDADD)

json_mapping_autodetect_SOURCES = json_mapping_autodetect.cpp
json_mapping_autodetect_LDADD = $(SPDOC_LDADD)

TESTS = $(EXTRA_PROGRAMS)

distclean-local:
	rm -rf $(TESTS)
