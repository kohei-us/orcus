SUBDIRS = include test parser mso liborcus spreadsheet

if BUILD_PYTHON
SUBDIRS += python
endif

AM_CPPFLAGS = \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/src/include

bin_PROGRAMS =

EXTRA_PROGRAMS = \
	orcus-test-xml \
	orcus-env-dump

AM_CPPFLAGS += $(BOOST_CPPFLAGS) $(LIBIXION_CFLAGS)

if HAVE_STATIC_LIB
AM_CPPFLAGS += -D__ORCUS_STATIC_LIB=1
endif

# orcus-test-xml

orcus_test_xml_SOURCES = orcus_test_xml.cpp
orcus_test_xml_LDADD = \
	parser/liborcus-parser-@ORCUS_API_VERSION@.la \
	liborcus/liborcus-@ORCUS_API_VERSION@.la

orcus_test_xml_CPPFLAGS = -I$(top_builddir)/lib/liborcus/liborcus.la $(AM_CPPFLAGS) -DSRCDIR=\""$(top_srcdir)"\"

# orcus-env-dump

orcus_env_dump_SOURCES = orcus_env_dump.cpp
orcus_env_dump_LDADD = \
	parser/liborcus-parser-@ORCUS_API_VERSION@.la \
	liborcus/liborcus-@ORCUS_API_VERSION@.la
orcus_env_dump_CPPFLAGS = -I$(top_builddir)/lib/liborcus/liborcus.la $(AM_CPPFLAGS)

TESTS = \
	orcus-test-xml \
	orcus-env-dump

if WITH_TOOLS

bin_PROGRAMS += \
	orcus-css-dump \
	orcus-zip-dump \
	orcus-mso-encryption \
	orcus-detect \
	orcus-json \
	orcus-yaml

# orcus-css-dump

orcus_css_dump_SOURCES = \
	orcus_css_dump.cpp

orcus_css_dump_LDADD = \
	parser/liborcus-parser-@ORCUS_API_VERSION@.la \
	liborcus/liborcus-@ORCUS_API_VERSION@.la

orcus_css_dump_CPPFLAGS = -I$(top_builddir)/lib/liborcus/liborcus.la $(AM_CPPFLAGS)

# orcus-zip-dump

orcus_zip_dump_SOURCES = \
	orcus_zip_dump.cpp

orcus_zip_dump_LDADD = \
	parser/liborcus-parser-@ORCUS_API_VERSION@.la

orcus_zip_dump_CPPFLAGS = $(AM_CPPFLAGS)

# orcus-json

orcus_json_SOURCES = \
	orcus_json_cli.hpp \
	orcus_json_cli.cpp \
	cli_global.hpp \
	cli_global.cpp \
	orcus_json_cli_map.cpp

orcus_json_LDFLAGS = \
	$(BOOST_PROGRAM_OPTIONS_LDFLAGS) \
	$(BOOST_FILESYSTEM_LDFLAGS) \
	$(BOOST_SYSTEM_LDFLAGS)

orcus_json_LDADD = \
	parser/liborcus-parser-@ORCUS_API_VERSION@.la \
	liborcus/liborcus-@ORCUS_API_VERSION@.la \
	$(BOOST_PROGRAM_OPTIONS_LIBS) \
	$(BOOST_FILESYSTEM_LIBS) \
	$(BOOST_SYSTEM_LIBS)

orcus_json_CPPFLAGS = -I$(top_builddir)/lib/liborcus/liborcus.la $(AM_CPPFLAGS)

# orcus-yaml

orcus_yaml_SOURCES = \
	orcus_yaml_main.cpp

orcus_yaml_LDFLAGS = \
	$(BOOST_PROGRAM_OPTIONS_LDFLAGS) \
	$(BOOST_FILESYSTEM_LDFLAGS) \
	$(BOOST_SYSTEM_LDFLAGS)

orcus_yaml_LDADD = \
	parser/liborcus-parser-@ORCUS_API_VERSION@.la \
	liborcus/liborcus-@ORCUS_API_VERSION@.la \
	$(BOOST_PROGRAM_OPTIONS_LIBS) \
	$(BOOST_FILESYSTEM_LIBS) \
	$(BOOST_SYSTEM_LIBS)

orcus_yaml_CPPFLAGS = -I$(top_builddir)/lib/liborcus/liborcus.la $(AM_CPPFLAGS)

#----------------------------------------------------------------------------

# orcus-mso-encryption

orcus_mso_encryption_SOURCES = orcus_mso_encryption.cpp
orcus_mso_encryption_LDFLAGS = $(BOOST_SYSTEM_LDFLAGS)
orcus_mso_encryption_LDADD = \
	parser/liborcus-parser-@ORCUS_API_VERSION@.la \
	mso/liborcus-mso-@ORCUS_API_VERSION@.la \
	$(BOOST_SYSTEM_LIBS)

orcus_mso_encryption_CPPFLAGS = -I$(top_builddir)/lib/liborcus/liborcus.la $(AM_CPPFLAGS)

#----------------------------------------------------------------------------

# orcus-detect

orcus_detect_SOURCES = orcus_detect_main.cpp
orcus_detect_LDFLAGS = \
	$(BOOST_PROGRAM_OPTIONS_LDFLAGS) \
	$(BOOST_FILESYSTEM_LDFLAGS) \
	$(BOOST_SYSTEM_LDFLAGS)
orcus_detect_LDADD = \
	parser/liborcus-parser-@ORCUS_API_VERSION@.la \
	liborcus/liborcus-@ORCUS_API_VERSION@.la \
	$(BOOST_PROGRAM_OPTIONS_LIBS) \
	$(BOOST_FILESYSTEM_LIBS) \
	$(BOOST_SYSTEM_LIBS)

orcus_detect_CPPFLAGS = -I$(top_builddir)/lib/liborcus/liborcus.la $(AM_CPPFLAGS)

#----------------------------------------------------------------------------

if BUILD_SPREADSHEET_MODEL

orcus_json_LDADD += \
	spreadsheet/liborcus-spreadsheet-model-@ORCUS_API_VERSION@.la

bin_PROGRAMS += \
	orcus-csv orcus-xml

# orcus-csv

orcus_csv_SOURCES = \
	orcus_filter_global.hpp \
	orcus_filter_global.cpp \
	orcus_csv_main.cpp

orcus_csv_LDFLAGS = \
	$(BOOST_PROGRAM_OPTIONS_LDFLAGS) \
	$(BOOST_FILESYSTEM_LDFLAGS) \
	$(BOOST_SYSTEM_LDFLAGS)
orcus_csv_LDADD = \
	liborcus/liborcus-@ORCUS_API_VERSION@.la \
	parser/liborcus-parser-@ORCUS_API_VERSION@.la \
	spreadsheet/liborcus-spreadsheet-model-@ORCUS_API_VERSION@.la \
	$(BOOST_PROGRAM_OPTIONS_LIBS) \
	$(BOOST_FILESYSTEM_LIBS) \
	$(BOOST_SYSTEM_LIBS)

orcus_csv_CPPFLAGS = -I$(top_builddir)/lib/liborcus/liborcus.la $(AM_CPPFLAGS)

# orcus-xml

orcus_xml_SOURCES = \
	orcus_filter_global.cpp \
	cli_global.hpp \
	cli_global.cpp \
	orcus_xml_main.cpp

orcus_xml_LDFLAGS = \
	$(BOOST_PROGRAM_OPTIONS_LDFLAGS) \
	$(BOOST_FILESYSTEM_LDFLAGS) \
	$(BOOST_SYSTEM_LDFLAGS)
orcus_xml_LDADD = \
	liborcus/liborcus-@ORCUS_API_VERSION@.la \
	parser/liborcus-parser-@ORCUS_API_VERSION@.la \
	spreadsheet/liborcus-spreadsheet-model-@ORCUS_API_VERSION@.la \
	$(BOOST_PROGRAM_OPTIONS_LIBS) \
	$(BOOST_FILESYSTEM_LIBS) \
	$(BOOST_SYSTEM_LIBS)

orcus_xml_CPPFLAGS = -I$(top_builddir)/lib/liborcus/liborcus.la $(AM_CPPFLAGS)

if WITH_ODS_FILTER

bin_PROGRAMS += \
	orcus-ods \
	orcus-styles-ods

# orcus-ods

orcus_ods_SOURCES = \
	orcus_filter_global.hpp \
	orcus_filter_global.cpp \
	orcus_ods_main.cpp

orcus_ods_LDFLAGS = \
	$(BOOST_PROGRAM_OPTIONS_LDFLAGS) \
	$(BOOST_FILESYSTEM_LDFLAGS) \
	$(BOOST_SYSTEM_LDFLAGS)
orcus_ods_LDADD = \
	liborcus/liborcus-@ORCUS_API_VERSION@.la \
	parser/liborcus-parser-@ORCUS_API_VERSION@.la \
	spreadsheet/liborcus-spreadsheet-model-@ORCUS_API_VERSION@.la \
	$(BOOST_PROGRAM_OPTIONS_LIBS) \
	$(BOOST_FILESYSTEM_LIBS) \
	$(BOOST_SYSTEM_LIBS)


orcus_ods_CPPFLAGS = -I$(top_builddir)/lib/liborcus/liborcus.la $(AM_CPPFLAGS)

# orcus-styles-ods

orcus_styles_ods_SOURCES = \
	orcus_filter_global.hpp \
	orcus_filter_global.cpp \
	orcus_ods_styles.cpp

orcus_styles_ods_LDFLAGS = \
	$(BOOST_PROGRAM_OPTIONS_LDFLAGS) \
	$(BOOST_FILESYSTEM_LDFLAGS) \
	$(BOOST_SYSTEM_LDFLAGS)

orcus_styles_ods_LDADD = \
	liborcus/liborcus-@ORCUS_API_VERSION@.la \
	parser/liborcus-parser-@ORCUS_API_VERSION@.la \
	spreadsheet/liborcus-spreadsheet-model-@ORCUS_API_VERSION@.la \
	$(BOOST_PROGRAM_OPTIONS_LIBS) \
	$(BOOST_FILESYSTEM_LIBS) \
	$(BOOST_SYSTEM_LIBS)

orcus_styles_ods_CPPFLAGS = -I$(top_builddir)/lib/liborcus/liborcus.la $(AM_CPPFLAGS)

endif # WITH_ODS_FILTER

if WITH_XLSX_FILTER

bin_PROGRAMS += \
	orcus-xlsx

# orcus-xlsx

orcus_xlsx_SOURCES = \
	orcus_filter_global.hpp \
	orcus_filter_global.cpp \
	orcus_xlsx_main.cpp

orcus_xlsx_LDFLAGS = \
	$(BOOST_PROGRAM_OPTIONS_LDFLAGS) \
	$(BOOST_FILESYSTEM_LDFLAGS) \
	$(BOOST_SYSTEM_LDFLAGS)
orcus_xlsx_LDADD = \
	liborcus/liborcus-@ORCUS_API_VERSION@.la \
	parser/liborcus-parser-@ORCUS_API_VERSION@.la \
	spreadsheet/liborcus-spreadsheet-model-@ORCUS_API_VERSION@.la \
	$(BOOST_PROGRAM_OPTIONS_LIBS) \
	$(BOOST_FILESYSTEM_LIBS) \
	$(BOOST_SYSTEM_LIBS)

orcus_xlsx_CPPFLAGS = -I$(top_builddir)/lib/liborcus/liborcus.la $(AM_CPPFLAGS)

endif # WITH_XLSX_FILTER

if WITH_XLS_XML_FILTER

bin_PROGRAMS += \
	orcus-xls-xml

# orcus-xls-xml

orcus_xls_xml_SOURCES = \
	orcus_filter_global.hpp \
	orcus_filter_global.cpp \
	orcus_xls_xml_main.cpp

orcus_xls_xml_LDFLAGS = \
	$(BOOST_PROGRAM_OPTIONS_LDFLAGS) \
	$(BOOST_FILESYSTEM_LDFLAGS) \
	$(BOOST_SYSTEM_LDFLAGS)
orcus_xls_xml_LDADD = \
	liborcus/liborcus-@ORCUS_API_VERSION@.la \
	parser/liborcus-parser-@ORCUS_API_VERSION@.la \
	spreadsheet/liborcus-spreadsheet-model-@ORCUS_API_VERSION@.la \
	$(BOOST_PROGRAM_OPTIONS_LIBS) \
	$(BOOST_FILESYSTEM_LIBS) \
	$(BOOST_SYSTEM_LIBS)

orcus_xls_xml_CPPFLAGS = -I$(top_builddir)/lib/liborcus/liborcus.la $(AM_CPPFLAGS)

endif # WITH_XLS_XML_FILTER

if WITH_GNUMERIC_FILTER

bin_PROGRAMS += \
	orcus-gnumeric

# orcus-gnumeric

orcus_gnumeric_SOURCES = \
	orcus_filter_global.hpp \
	orcus_filter_global.cpp \
	orcus_gnumeric_main.cpp

orcus_gnumeric_LDFLAGS = \
	$(BOOST_PROGRAM_OPTIONS_LDFLAGS) \
	$(BOOST_FILESYSTEM_LDFLAGS) \
	$(BOOST_SYSTEM_LDFLAGS)
orcus_gnumeric_LDADD = \
	liborcus/liborcus-@ORCUS_API_VERSION@.la \
	parser/liborcus-parser-@ORCUS_API_VERSION@.la \
	spreadsheet/liborcus-spreadsheet-model-@ORCUS_API_VERSION@.la \
	$(BOOST_PROGRAM_OPTIONS_LIBS) \
	$(BOOST_FILESYSTEM_LIBS) \
	$(BOOST_SYSTEM_LIBS)

orcus_gnumeric_CPPFLAGS = -I$(top_builddir)/lib/liborcus/liborcus.la $(AM_CPPFLAGS)

endif # WITH_GNUMERIC_FILTER

if WITH_PARQUET_FILTER

bin_PROGRAMS += \
	orcus-parquet

# orcus-parquet

orcus_parquet_SOURCES = \
	orcus_filter_global.hpp \
	orcus_filter_global.cpp \
	orcus_parquet_main.cpp

orcus_parquet_LDFLAGS = \
	$(BOOST_PROGRAM_OPTIONS_LDFLAGS) \
	$(BOOST_FILESYSTEM_LDFLAGS) \
	$(BOOST_SYSTEM_LDFLAGS)
orcus_parquet_LDADD = \
	liborcus/liborcus-@ORCUS_API_VERSION@.la \
	parser/liborcus-parser-@ORCUS_API_VERSION@.la \
	spreadsheet/liborcus-spreadsheet-model-@ORCUS_API_VERSION@.la \
	$(BOOST_PROGRAM_OPTIONS_LIBS) \
	$(BOOST_FILESYSTEM_LIBS) \
	$(BOOST_SYSTEM_LIBS)

endif # WITH_PARQUET_FILTER

endif # BUILD_SPREADSHEET_MODEL

endif # WITH_TOOLS

#----------------------------------------------------------------------------
# Orcus Filter Tests
#----------------------------------------------------------------------------

if BUILD_SPREADSHEET_MODEL

EXTRA_PROGRAMS += \
	orcus-test-csv \
	orcus-test-xml-mapped \
	orcus-test-json-mapped

# orcus-test-csv

orcus_test_csv_SOURCES = \
	orcus_test_csv.cpp \
	orcus_test_global.hpp \
	orcus_test_global.cpp

orcus_test_csv_LDADD = \
	liborcus/liborcus-@ORCUS_API_VERSION@.la \
	parser/liborcus-parser-@ORCUS_API_VERSION@.la \
	./test/liborcus-test.a \
	spreadsheet/liborcus-spreadsheet-model-@ORCUS_API_VERSION@.la

orcus_test_csv_CPPFLAGS = -I$(top_builddir)/lib/liborcus/liborcus.la $(AM_CPPFLAGS) -DSRCDIR=\""$(top_srcdir)"\"

# orcus-test-xml-mapped

orcus_test_xml_mapped_SOURCES = \
	orcus_test_xml_mapped.cpp \
	orcus_test_global.hpp \
	orcus_test_global.cpp

orcus_test_xml_mapped_LDFLAGS = \
	$(BOOST_FILESYSTEM_LDFLAGS) \
	$(BOOST_SYSTEM_LDFLAGS)

orcus_test_xml_mapped_LDADD = \
	liborcus/liborcus-@ORCUS_API_VERSION@.la \
	parser/liborcus-parser-@ORCUS_API_VERSION@.la \
	./test/liborcus-test.a \
	spreadsheet/liborcus-spreadsheet-model-@ORCUS_API_VERSION@.la \
	$(BOOST_FILESYSTEM_LIBS) \
	$(BOOST_SYSTEM_LIBS)

orcus_test_xml_mapped_CPPFLAGS = -I$(top_builddir)/lib/liborcus/liborcus.la $(AM_CPPFLAGS) -DSRCDIR=\""$(top_srcdir)"\"

# orcus-test-json-mapped

orcus_test_json_mapped_SOURCES = \
	orcus_test_json_mapped.cpp

orcus_test_json_mapped_LDFLAGS = \
	$(BOOST_FILESYSTEM_LDFLAGS) \
	$(BOOST_SYSTEM_LDFLAGS)

orcus_test_json_mapped_LDADD = \
	liborcus/liborcus-@ORCUS_API_VERSION@.la \
	parser/liborcus-parser-@ORCUS_API_VERSION@.la \
	spreadsheet/liborcus-spreadsheet-model-@ORCUS_API_VERSION@.la \
	$(BOOST_FILESYSTEM_LIBS) \
	$(BOOST_SYSTEM_LIBS)

orcus_test_json_mapped_CPPFLAGS = -I$(top_builddir)/lib/liborcus/liborcus.la $(AM_CPPFLAGS) -DSRCDIR=\""$(top_srcdir)"\"

TESTS += \
	orcus-test-csv \
	orcus-test-xml-mapped \
	orcus-test-json-mapped

if WITH_ODS_FILTER

EXTRA_PROGRAMS += \
	orcus-test-ods

# orcus-test-ods

orcus_test_ods_SOURCES = \
	orcus_test_ods.cpp

orcus_test_ods_LDADD = \
	liborcus/liborcus-@ORCUS_API_VERSION@.la \
	parser/liborcus-parser-@ORCUS_API_VERSION@.la \
	spreadsheet/liborcus-spreadsheet-model-@ORCUS_API_VERSION@.la \
	$(BOOST_FILESYSTEM_LIBS) \
	$(BOOST_SYSTEM_LIBS)

orcus_test_ods_LDFLAGS = \
	$(BOOST_FILESYSTEM_LDFLAGS) \
	$(BOOST_SYSTEM_LDFLAGS)

orcus_test_ods_CPPFLAGS = $(AM_CPPFLAGS) \
	-I$(top_builddir)/lib/liborcus/liborcus.la -DSRCDIR=\""$(top_srcdir)"\"

TESTS += orcus-test-ods

# orcus-test-import-ods

EXTRA_PROGRAMS += \
	orcus-test-import-ods

orcus_test_import_ods_SOURCES = \
	orcus_test_import_ods.cpp

orcus_test_import_ods_LDADD = \
	liborcus/liborcus-@ORCUS_API_VERSION@.la \
	parser/liborcus-parser-@ORCUS_API_VERSION@.la \
	spreadsheet/liborcus-spreadsheet-model-@ORCUS_API_VERSION@.la \
	./test/liborcus-test.a \
	$(BOOST_FILESYSTEM_LIBS) \
	$(BOOST_SYSTEM_LIBS)

orcus_test_import_ods_LDFLAGS = \
	$(BOOST_FILESYSTEM_LDFLAGS) \
	$(BOOST_SYSTEM_LDFLAGS)

orcus_test_import_ods_CPPFLAGS = $(AM_CPPFLAGS) \
	-I$(top_builddir)/lib/liborcus/liborcus.la -DSRCDIR=\""$(top_srcdir)"\"

TESTS += \
	orcus-test-import-ods

endif # WITH_ODS_FILTER

if WITH_XLSX_FILTER

EXTRA_PROGRAMS += \
	orcus-test-xlsx

# orcus-test-xlsx

orcus_test_xlsx_SOURCES = \
	orcus_test_xlsx.cpp

orcus_test_xlsx_LDFLAGS = \
	$(BOOST_FILESYSTEM_LDFLAGS) \
	$(BOOST_SYSTEM_LDFLAGS)

orcus_test_xlsx_LDADD = \
	liborcus/liborcus-@ORCUS_API_VERSION@.la \
	parser/liborcus-parser-@ORCUS_API_VERSION@.la \
	test/liborcus-test.a \
	spreadsheet/liborcus-spreadsheet-model-@ORCUS_API_VERSION@.la \
	@LIBIXION_LIBS@ \
	$(BOOST_FILESYSTEM_LIBS) \
	$(BOOST_SYSTEM_LIBS)

orcus_test_xlsx_CPPFLAGS = \
	@LIBIXION_CFLAGS@  $(AM_CPPFLAGS) \
	-I$(top_builddir)/lib/liborcus/liborcus.la -DSRCDIR=\""$(top_srcdir)"\"

TESTS += \
	 orcus-test-xlsx

endif # WITH_XLSX_FILTER

if WITH_XLS_XML_FILTER

EXTRA_PROGRAMS += orcus-test-xls-xml

# orcus-test-xls-xml

orcus_test_xls_xml_SOURCES = \
	orcus_test_xls_xml.cpp \
	orcus_test_global.cpp

orcus_test_xls_xml_LDADD = \
	liborcus/liborcus-@ORCUS_API_VERSION@.la \
	parser/liborcus-parser-@ORCUS_API_VERSION@.la \
	./test/liborcus-test.a \
	spreadsheet/liborcus-spreadsheet-model-@ORCUS_API_VERSION@.la \
	@LIBIXION_LIBS@

orcus_test_xls_xml_CPPFLAGS = \
	@LIBIXION_CFLAGS@  $(AM_CPPFLAGS) \
	-I$(top_builddir)/lib/liborcus/liborcus.la -DSRCDIR=\""$(top_srcdir)"\"

TESTS += \
	 orcus-test-xls-xml

endif # WITH_XLS_XML_FILTER

if WITH_GNUMERIC_FILTER

EXTRA_PROGRAMS += orcus-test-gnumeric

orcus_test_gnumeric_SOURCES = \
	orcus_test_gnumeric.cpp \
	orcus_test_global.cpp

orcus_test_gnumeric_LDADD = \
	liborcus/liborcus-@ORCUS_API_VERSION@.la \
	parser/liborcus-parser-@ORCUS_API_VERSION@.la \
	./test/liborcus-test.a \
	spreadsheet/liborcus-spreadsheet-model-@ORCUS_API_VERSION@.la

orcus_test_gnumeric_CPPFLAGS = \
	@LIBIXION_CFLAGS@  $(AM_CPPFLAGS) \
	-I$(top_builddir)/lib/liborcus/liborcus.la -DSRCDIR=\""$(top_srcdir)"\"

TESTS += orcus-test-gnumeric

endif # WITH_GNUMERIC_FILTER

endif # BUILD_SPREADSHEET_MODEL

distclean-local:
	rm -rf $(TESTS)

@VALGRIND_CHECK_RULES@
