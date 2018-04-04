if BUILD_SPREADSHEET_MODEL

AM_CPPFLAGS = -I$(top_srcdir)/include -D__ORCUS_SPM_BUILDING_DLL

AM_CPPFLAGS += $(BOOST_CPPFLAGS) $(LIBIXION_CFLAGS)

COMMON_CPPFLAGS = $(AM_CPPFLAGS)

if HAVE_STATIC_LIB
AM_CPPFLAGS += -D__ORCUS_STATIC_LIB=1
endif

lib_LTLIBRARIES = liborcus-spreadsheet-model-@ORCUS_API_VERSION@.la
liborcus_spreadsheet_model_@ORCUS_API_VERSION@_la_SOURCES = \
	auto_filter.cpp \
	config.cpp \
	document.cpp \
	dumper_global.hpp \
	dumper_global.cpp \
	factory.cpp \
	factory_pivot.hpp \
	factory_pivot.cpp \
	factory_sheet.hpp \
	factory_sheet.cpp \
	factory_styles.cpp \
	factory_table.hpp \
	factory_table.cpp \
	formula_global.hpp \
	formula_global.cpp \
	csv_dumper.hpp \
	csv_dumper.cpp \
	json_dumper.hpp \
	json_dumper.cpp \
	pivot.cpp \
	shared_formula.hpp \
	shared_formula.cpp \
	shared_strings.cpp \
	sheet.cpp \
	sheet_range.cpp \
	styles.cpp \
	view.cpp \
	global_settings.cpp

liborcus_spreadsheet_model_@ORCUS_API_VERSION@_la_CPPFLAGS = $(AM_CPPFLAGS) $(LIBIXION_CFLAGS)
liborcus_spreadsheet_model_@ORCUS_API_VERSION@_la_LDFLAGS = -no-undefined
liborcus_spreadsheet_model_@ORCUS_API_VERSION@_la_LIBADD = $(LIBIXION_LIBS) $(BOOST_DATE_TIME_LIBS) \
	../parser/liborcus-parser-@ORCUS_API_VERSION@.la \
	../liborcus/liborcus-@ORCUS_API_VERSION@.la

endif
