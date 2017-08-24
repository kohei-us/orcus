
AM_CPPFLAGS = -I$(top_srcdir)/include

bin_PROGRAMS =

EXTRA_PROGRAMS = \
	json-doc-1 \
	json-doc-2 \
	json-parser-1

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

TESTS = \
	json-doc-1 \
	json-doc-2 \
	json-parser-1

distclean-local:
	rm -rf $(TESTS)
