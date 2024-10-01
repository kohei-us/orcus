if BUILD_SPREADSHEET_MODEL

AM_CPPFLAGS = \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/src/include \
	-D__ORCUS_SPM_BUILDING_DLL

AM_CPPFLAGS += $(BOOST_CPPFLAGS) $(LIBIXION_CFLAGS)

EXTRA_PROGRAMS = \
	auto-filter-test

TESTS = \
	auto-filter-test

if HAVE_FILESYSTEM
AM_CPPFLAGS += "-DHAVE_FILESYSTEM=1"
endif

if HAVE_EXPERIMENTAL_FILESYSTEM
AM_CPPFLAGS += "-DHAVE_EXPERIMENTAL_FILESYSTEM=1"
endif

COMMON_CPPFLAGS = $(AM_CPPFLAGS)

if HAVE_STATIC_LIB
AM_CPPFLAGS += -D__ORCUS_STATIC_LIB=1
endif

lib_LTLIBRARIES = liborcus-spreadsheet-model-@ORCUS_API_VERSION@.la
liborcus_spreadsheet_model_@ORCUS_API_VERSION@_la_SOURCES = \
	auto_filter.cpp \
	check_dumper.hpp \
	check_dumper.cpp \
	config.cpp \
	debug_state_dumper.hpp \
	debug_state_dumper.cpp \
	document.cpp \
	document_impl.hpp \
	document_impl.cpp \
	document_types.cpp \
	dumper_global.hpp \
	dumper_global.cpp \
	factory.cpp \
	factory_auto_filter.hpp \
	factory_auto_filter.cpp \
	factory_pivot.hpp \
	factory_pivot.cpp \
	factory_shared_strings.hpp \
	factory_shared_strings.cpp \
	factory_sheet.hpp \
	factory_sheet.cpp \
	factory_strikethrough.hpp \
	factory_strikethrough.cpp \
	factory_styles.cpp \
	factory_table.hpp \
	factory_table.cpp \
	factory_underline.hpp \
	factory_underline.cpp \
	flat_dumper.hpp \
	flat_dumper.cpp \
	formula_global.hpp \
	formula_global.cpp \
	html_dumper.hpp \
	html_dumper.cpp \
	impl_types.hpp \
	csv_dumper.hpp \
	csv_dumper.cpp \
	json_dumper.hpp \
	json_dumper.cpp \
	number_format.hpp \
	number_format.cpp \
	pivot.cpp \
	shared_formula.hpp \
	shared_formula.cpp \
	shared_strings.cpp \
	sheet.cpp \
	sheet_impl.hpp \
	sheet_impl.cpp \
	styles.cpp \
	table.cpp \
	tables.cpp \
	view.cpp \
	global_settings.hpp \
	global_settings.cpp

liborcus_spreadsheet_model_@ORCUS_API_VERSION@_la_CPPFLAGS = $(AM_CPPFLAGS) $(LIBIXION_CFLAGS)
liborcus_spreadsheet_model_@ORCUS_API_VERSION@_la_LDFLAGS = -no-undefined
liborcus_spreadsheet_model_@ORCUS_API_VERSION@_la_LIBADD = \
	$(LIBIXION_LIBS) \
	$(BOOST_DATE_TIME_LIBS) \
	$(BOOST_SYSTEM_LIBS) \
	../parser/liborcus-parser-@ORCUS_API_VERSION@.la \
	../liborcus/liborcus-@ORCUS_API_VERSION@.la

if !HAVE_FILESYSTEM
if HAVE_EXPERIMENTAL_FILESYSTEM
liborcus_spreadsheet_model_@ORCUS_API_VERSION@_la_LIBADD += -lstdc++fs
else
liborcus_spreadsheet_model_@ORCUS_API_VERSION@_la_LIBADD += $(BOOST_FILESYSTEM_LIBS)
endif
endif

# auto-filter-test

auto_filter_test_SOURCES = \
	auto_filter_test.cpp \
	auto_filter.cpp

auto_filter_test_LDADD = \
	$(LIBIXION_LIBS) \
	../test/liborcus-test.a \
	../parser/liborcus-parser-@ORCUS_API_VERSION@.la

endif # BUILD_SPREADSHEET_MODEL
