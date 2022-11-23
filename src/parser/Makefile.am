
AM_CPPFLAGS = \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/src/include \
	-DSRCDIR=\""$(top_srcdir)"\" \
	$(BOOST_CPPFLAGS) \
	-D__ORCUS_PSR_BUILDING_DLL

lib_LTLIBRARIES = liborcus-parser-@ORCUS_API_VERSION@.la
liborcus_parser_@ORCUS_API_VERSION@_la_SOURCES = \
	win_stdint.h \
	base64.cpp \
	cell_buffer.cpp \
	css_parser_base.cpp \
	css_types.cpp \
	csv_parser_base.cpp \
	exception.cpp \
	json_global.cpp \
	json_parser_base.cpp \
	json_parser_thread.cpp \
	parser_base.cpp \
	parser_global.cpp \
	pstring.cpp \
	sax_parser_base.cpp \
	sax_token_parser.cpp \
	sax_token_parser_thread.cpp \
	stream.cpp \
	string_pool.cpp \
	tokens.cpp \
	types.cpp \
	utf8.hpp \
	utf8.cpp \
	xml_namespace.cpp \
	xml_writer.cpp \
	yaml_parser_base.cpp \
	zip_archive.cpp \
	zip_archive_stream.cpp


liborcus_parser_@ORCUS_API_VERSION@_la_LDFLAGS = \
	-no-undefined \
	$(BOOST_SYSTEM_LDFLAGS) \
	$(BOOST_FILESYSTEM_LDFLAGS)

liborcus_parser_@ORCUS_API_VERSION@_la_LIBADD = \
	$(BOOST_SYSTEM_LIBS) \
	$(BOOST_FILESYSTEM_LIBS) \
	$(ZLIB_LIBS)

EXTRA_PROGRAMS = \
	css-parser-test \
	csv-parser-test \
	json-parser-test \
	parser-global-test \
	parser-test-base \
	parser-test-base64 \
	parser-test-json-validation \
	parser-test-numeric \
	parser-test-stream \
	parser-test-string-pool \
	parser-test-threaded-json-parser \
	parser-test-threaded-sax-token-parser \
	parser-test-xml-namespace \
	parser-test-xml-validation \
	parser-test-zip-archive \
	sax-ns-parser-test \
	sax-parser-test \
	sax-token-parser-test \
	types-test \
	utf8-test \
	yaml-parser-test \
	xml-writer-test

# parser-test-string-pool

parser_test_string_pool_SOURCES = \
	string_pool.cpp \
	string_pool_test.cpp

parser_test_string_pool_LDADD = liborcus-parser-@ORCUS_API_VERSION@.la
parser_test_string_pool_CPPFLAGS = $(AM_CPPFLAGS)

# parser-test-xml-namespace

parser_test_xml_namespace_SOURCES = \
	xml_namespace.cpp \
	xml_namespace_test.cpp

parser_test_xml_namespace_LDADD = liborcus-parser-@ORCUS_API_VERSION@.la
parser_test_xml_namespace_CPPFLAGS = $(AM_CPPFLAGS)

# parser-test-xml-validation

parser_test_xml_validation_SOURCES = \
	parser_test_xml_validation.cpp

parser_test_xml_validation_CPPFLAGS = $(AM_CPPFLAGS)
parser_test_xml_validation_LDFLAGS = \
	$(BOOST_SYSTEM_LDFLAGS) \
	$(BOOST_FILESYSTEM_LDFLAGS)
parser_test_xml_validation_LDADD = \
	liborcus-parser-@ORCUS_API_VERSION@.la \
	$(BOOST_SYSTEM_LIBS) \
	$(BOOST_FILESYSTEM_LIBS)

# parser-test-base64

parser_test_base64_SOURCES = \
	base64.cpp \
	base64_test.cpp

parser_test_base64_LDADD = liborcus-parser-@ORCUS_API_VERSION@.la
parser_test_base64_CPPFLAGS = $(AM_CPPFLAGS)

# css-parser-test

css_parser_test_SOURCES = \
	css_parser_test.cpp

css_parser_test_LDADD = liborcus-parser-@ORCUS_API_VERSION@.la
css_parser_test_CPPFLAGS = $(AM_CPPFLAGS)

# csv-parser-test

csv_parser_test_SOURCES = \
	csv_parser_test.cpp

csv_parser_test_LDADD = liborcus-parser-@ORCUS_API_VERSION@.la
csv_parser_test_CPPFLAGS = $(AM_CPPFLAGS)

# json-parser-test

json_parser_test_SOURCES = \
	json_parser_test.cpp

json_parser_test_LDADD = liborcus-parser-@ORCUS_API_VERSION@.la
json_parser_test_CPPFLAGS = $(AM_CPPFLAGS)

# yaml-parser-test

yaml_parser_test_SOURCES = \
	yaml_parser_test.cpp

yaml_parser_test_LDADD = liborcus-parser-@ORCUS_API_VERSION@.la
yaml_parser_test_CPPFLAGS = $(AM_CPPFLAGS)

# sax-parser-test

sax_parser_test_SOURCES = \
	sax_parser_test.cpp

sax_parser_test_LDADD = liborcus-parser-@ORCUS_API_VERSION@.la
sax_parser_test_CPPFLAGS = $(AM_CPPFLAGS)

# sax-ns-parser-test

sax_ns_parser_test_SOURCES = \
	sax_ns_parser_test.cpp

sax_ns_parser_test_LDADD = liborcus-parser-@ORCUS_API_VERSION@.la
sax_ns_parser_test_CPPFLAGS = $(AM_CPPFLAGS)

# sax-token-parser-test

sax_token_parser_test_SOURCES = \
	sax_token_parser_test.cpp

sax_token_parser_test_LDADD = liborcus-parser-@ORCUS_API_VERSION@.la
sax_token_parser_test_CPPFLAGS = $(AM_CPPFLAGS)

# parser-test-threaded-sax-token-parser

parser_test_threaded_sax_token_parser_SOURCES = \
	threaded_sax_token_parser_test.cpp

parser_test_threaded_sax_token_parser_LDADD = liborcus-parser-@ORCUS_API_VERSION@.la
parser_test_threaded_sax_token_parser_CPPFLAGS = $(AM_CPPFLAGS)

# parser-test-threaded-json-parser

parser_test_threaded_json_parser_SOURCES = \
	threaded_json_parser_test.cpp

parser_test_threaded_json_parser_LDADD = liborcus-parser-@ORCUS_API_VERSION@.la
parser_test_threaded_json_parser_LDFLAGS = -pthread
parser_test_threaded_json_parser_CPPFLAGS = $(AM_CPPFLAGS)

# parser-test-stream

parser_test_stream_SOURCES = \
	stream_test.cpp

parser_test_stream_LDADD = \
	liborcus-parser-@ORCUS_API_VERSION@.la \
	../test/liborcus-test.a
parser_test_stream_CPPFLAGS = $(AM_CPPFLAGS)

# parser-test-zip-archive

parser_test_zip_archive_SOURCES = \
	zip_archive_test.cpp

parser_test_zip_archive_CPPFLAGS = $(AM_CPPFLAGS)
parser_test_zip_archive_LDADD = \
	liborcus-parser-@ORCUS_API_VERSION@.la \
	../test/liborcus-test.a \
	$(BOOST_SYSTEM_LIBS) \
	$(BOOST_FILESYSTEM_LIBS)
parser_test_zip_archive_LDFLAGS = \
	$(BOOST_SYSTEM_LDFLAGS) \
	$(BOOST_FILESYSTEM_LDFLAGS)

# parser-test-base

parser_test_base_SOURCES = \
	parser_base_test.cpp

parser_test_base_LDADD = liborcus-parser-@ORCUS_API_VERSION@.la
parser_test_base_CPPFLAGS = $(AM_CPPFLAGS)

# parser-global-test

parser_global_test_SOURCES = \
	parser_global_test.cpp

parser_global_test_LDADD = \
	liborcus-parser-@ORCUS_API_VERSION@.la \
	../test/liborcus-test.a
parser_global_test_CPPFLAGS = $(AM_CPPFLAGS)

# parser-test-json-validation

parser_test_json_validation_SOURCES = \
	parser_test_json_validation.cpp

parser_test_json_validation_LDADD = liborcus-parser-@ORCUS_API_VERSION@.la
parser_test_json_validation_CPPFLAGS = $(AM_CPPFLAGS)

# types-test

types_test_SOURCES = types_test.cpp

types_test_LDADD = \
	liborcus-parser-@ORCUS_API_VERSION@.la \
	../test/liborcus-test.a
types_test_CPPFLAGS = $(AM_CPPFLAGS)

# utf8-test

utf8_test_SOURCES = \
	utf8.cpp \
	utf8_test.cpp

utf8_test_LDADD = liborcus-parser-@ORCUS_API_VERSION@.la
utf8_test_CPPFLAGS = $(AM_CPPFLAGS)

# xml-writer-test

xml_writer_test_SOURCES = xml_writer_test.cpp

xml_writer_test_LDADD = liborcus-parser-@ORCUS_API_VERSION@.la
xml_writer_test_CPPFLAGS = $(AM_CPPFLAGS)

# parser-test-numeric

parser_test_numeric_SOURCES = \
	parser_test_numeric.cpp

parser_test_numeric = liborcus-parser-@ORCUS_API_VERSION@.la
parser_test_numeric_LDADD = liborcus-parser-@ORCUS_API_VERSION@.la
parser_test_numeric_CPPFLAGS = $(AM_CPPFLAGS)

TESTS = \
	css-parser-test \
	csv-parser-test \
	json-parser-test \
	parser-global-test \
	parser-test-base \
	parser-test-base64 \
	parser-test-json-validation \
	parser-test-numeric \
	parser-test-stream \
	parser-test-string-pool \
	parser-test-threaded-json-parser \
	parser-test-threaded-sax-token-parser \
	parser-test-xml-namespace \
	parser-test-xml-validation \
	parser-test-zip-archive \
	sax-ns-parser-test \
	sax-parser-test \
	sax-token-parser-test \
	types-test \
	utf8-test \
	xml-writer-test

distclean-local:
	rm -rf $(TESTS)

@VALGRIND_CHECK_RULES@
