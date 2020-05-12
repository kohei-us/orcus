SUBDIRS = .

AM_CPPFLAGS = -I$(top_srcdir)/include

bin_PROGRAMS =

EXTRA_PROGRAMS = \
	orcus-handler-test-csv \
	orcus-handler-test-json \
	orcus-handler-test-yaml

# orcus-handler-test-csv

orcus_handler_test_csv_SOURCES = \
	csv.hpp \
	csv_test.cpp

orcus_handler_test_csv_LDADD = \
	../src/parser/liborcus-parser-@ORCUS_API_VERSION@.la

orcus_handler_test_csv_CPPFLAGS = $(AM_CPPFLAGS)

# orcus-handler-test-json

orcus_handler_test_json_SOURCES = \
	json.hpp \
	json_test.cpp

orcus_handler_test_json_LDADD = \
	../src/parser/liborcus-parser-@ORCUS_API_VERSION@.la

# orcus-handler-test-yaml

orcus_handler_test_yaml_SOURCES = \
	yaml.hpp \
	yaml_test.cpp

orcus_handler_test_yaml_LDADD = \
	../src/parser/liborcus-parser-@ORCUS_API_VERSION@.la

TESTS = \
	orcus-handler-test-csv \
	orcus-handler-test-json \
	orcus-handler-test-yaml


distclean-local:
	rm -rf $(TESTS)

