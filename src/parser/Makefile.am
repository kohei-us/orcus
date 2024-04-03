
AM_CPPFLAGS = \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/src/include \
	-DSRCDIR=\""$(top_srcdir)"\" \
	$(BOOST_CPPFLAGS) \
	-D__ORCUS_PSR_BUILDING_DLL

if HAVE_FILESYSTEM
AM_CPPFLAGS += "-DHAVE_FILESYSTEM=1"
endif

if HAVE_EXPERIMENTAL_FILESYSTEM
AM_CPPFLAGS += "-DHAVE_EXPERIMENTAL_FILESYSTEM=1"
endif

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
	$(BOOST_SYSTEM_LDFLAGS)

liborcus_parser_@ORCUS_API_VERSION@_la_LIBADD = \
	$(BOOST_SYSTEM_LIBS) \
	$(ZLIB_LIBS)

if !HAVE_FILESYSTEM
if HAVE_EXPERIMENTAL_FILESYSTEM
liborcus_parser_@ORCUS_API_VERSION@_la_LDFLAGS += -lstdc++fs
else
liborcus_parser_@ORCUS_API_VERSION@_la_LDFLAGS += $(BOOST_FILESYSTEM_LDFLAGS)
liborcus_parser_@ORCUS_API_VERSION@_la_LIBADD += $(BOOST_FILESYSTEM_LIBS)
endif
endif

EXTRA_PROGRAMS = \
	base64-test \
	css-parser-test \
	csv-parser-test \
	json-parser-test \
	json-validation-test \
	numeric-parser-test \
	parser-base-test \
	parser-global-test \
	sax-ns-parser-test \
	sax-parser-test \
	sax-token-parser-test \
	stream-test \
	string-pool-test \
	threaded-json-parser-test \
	threaded-sax-token-parser-test \
	types-test \
	utf8-test \
	xml-namespace-test \
	xml-validation-test \
	xml-writer-test \
	yaml-parser-test \
	zip-archive-test

# string-pool-test

string_pool_test_SOURCES = \
	string_pool.cpp \
	string_pool_test.cpp

string_pool_test_LDADD = \
    liborcus-parser-@ORCUS_API_VERSION@.la \
	$(BOOST_SYSTEM_LIBS)

string_pool_test_CPPFLAGS = $(AM_CPPFLAGS)

# xml-namespace-test

xml_namespace_test_SOURCES = \
	xml_namespace.cpp \
	xml_namespace_test.cpp

xml_namespace_test_LDADD = \
	liborcus-parser-@ORCUS_API_VERSION@.la \
	../test/liborcus-test.a \
	$(BOOST_SYSTEM_LIBS)

xml_namespace_test_CPPFLAGS = $(AM_CPPFLAGS)

# xml-validation-test

xml_validation_test_SOURCES = \
	xml_validation_test.cpp

xml_validation_test_CPPFLAGS = $(AM_CPPFLAGS)
xml_validation_test_LDFLAGS = \
	$(BOOST_SYSTEM_LDFLAGS)
xml_validation_test_LDADD = \
	liborcus-parser-@ORCUS_API_VERSION@.la \
	../test/liborcus-test.a \
	$(BOOST_SYSTEM_LIBS)

if !HAVE_FILESYSTEM
if HAVE_EXPERIMENTAL_FILESYSTEM
xml_validation_test_LDFLAGS += -lstdc++fs
else
xml_validation_test_LDFLAGS += $(BOOST_FILESYSTEM_LDFLAGS)
xml_validation_test_LDADD += $(BOOST_FILESYSTEM_LIBS)
endif
endif

# base64-test

base64_test_SOURCES = \
	base64.cpp \
	base64_test.cpp

base64_test_LDADD = liborcus-parser-@ORCUS_API_VERSION@.la
base64_test_CPPFLAGS = $(AM_CPPFLAGS)

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

# threaded-sax-token-parser-test

threaded_sax_token_parser_test_SOURCES = \
	threaded_sax_token_parser_test.cpp

threaded_sax_token_parser_test_LDADD = liborcus-parser-@ORCUS_API_VERSION@.la
threaded_sax_token_parser_test_CPPFLAGS = $(AM_CPPFLAGS)

# threaded-json-parser-test

threaded_json_parser_test_SOURCES = \
	threaded_json_parser_test.cpp

threaded_json_parser_test_LDADD = liborcus-parser-@ORCUS_API_VERSION@.la
threaded_json_parser_test_LDFLAGS = -pthread
threaded_json_parser_test_CPPFLAGS = $(AM_CPPFLAGS)

# stream-test

stream_test_SOURCES = \
	stream_test.cpp

stream_test_LDADD = \
	liborcus-parser-@ORCUS_API_VERSION@.la \
	../test/liborcus-test.a
stream_test_CPPFLAGS = $(AM_CPPFLAGS)

# zip-archive-test

zip_archive_test_SOURCES = \
	zip_archive_test.cpp

zip_archive_test_CPPFLAGS = $(AM_CPPFLAGS)
zip_archive_test_LDADD = \
	liborcus-parser-@ORCUS_API_VERSION@.la \
	../test/liborcus-test.a \
	$(BOOST_SYSTEM_LIBS)
zip_archive_test_LDFLAGS = \
	$(BOOST_SYSTEM_LDFLAGS)

if !HAVE_FILESYSTEM
if HAVE_EXPERIMENTAL_FILESYSTEM
zip_archive_test_LDFLAGS += -lstdc++fs
else
zip_archive_test_LDFLAGS += $(BOOST_FILESYSTEM_LDFLAGS)
zip_archive_test_LDADD += $(BOOST_FILESYSTEM_LIBS)
endif
endif



# parser-base-test

parser_base_test_SOURCES = \
	parser_base_test.cpp

parser_base_test_LDADD = liborcus-parser-@ORCUS_API_VERSION@.la
parser_base_test_CPPFLAGS = $(AM_CPPFLAGS)

# parser-global-test

parser_global_test_SOURCES = \
	parser_global_test.cpp

parser_global_test_LDADD = \
	liborcus-parser-@ORCUS_API_VERSION@.la \
	../test/liborcus-test.a
parser_global_test_CPPFLAGS = $(AM_CPPFLAGS)

# json-validation-test

json_validation_test_SOURCES = \
	json_validation_test.cpp

json_validation_test_LDADD = liborcus-parser-@ORCUS_API_VERSION@.la
json_validation_test_CPPFLAGS = $(AM_CPPFLAGS)

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

# numeric-parser-test

numeric_parser_test_SOURCES = \
	numeric_parser_test.cpp

numeric_parser_test = liborcus-parser-@ORCUS_API_VERSION@.la
numeric_parser_test_LDADD = liborcus-parser-@ORCUS_API_VERSION@.la
numeric_parser_test_CPPFLAGS = $(AM_CPPFLAGS)

TESTS = \
	base64-test \
	css-parser-test \
	csv-parser-test \
	json-parser-test \
	json-validation-test \
	numeric-parser-test \
	parser-base-test \
	parser-global-test \
	sax-ns-parser-test \
	sax-parser-test \
	sax-token-parser-test \
	stream-test \
	string-pool-test \
	threaded-json-parser-test \
	threaded-sax-token-parser-test \
	types-test \
	utf8-test \
	xml-namespace-test \
	xml-validation-test \
	xml-writer-test \
	yaml-parser-test \
	zip-archive-test

distclean-local:
	rm -rf $(TESTS)

@VALGRIND_CHECK_RULES@
