
AM_CPPFLAGS = \
	-I$(top_srcdir)/include \
	$(LIBIXION_CFLAGS) \
	-DSRCDIR=\""$(top_srcdir)"\"

EXTRA_PROGRAMS = \
	xml-namespace

xml_namespace_SOURCES = \
	xml_namespace.cpp

xml_namespace_LDADD = \
	../../src/parser/liborcus-parser-@ORCUS_API_VERSION@.la \
	../../src/liborcus/liborcus-@ORCUS_API_VERSION@.la

TESTS = $(EXTRA_PROGRAMS)

distclean-local:
	rm -rf $(TESTS)
