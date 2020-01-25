if BUILD_PYTHON

AM_CPPFLAGS = $(BOOST_CPPFLAGS) $(PYTHON_CFLAGS) $(MDDS_CFLAGS) $(LIBIXION_CFLAGS)

pyexec_LTLIBRARIES = _orcus.la _orcus_json.la

_orcus_la_SOURCES = \
	python.cpp \
	global.hpp \
	global.cpp \
	memory.hpp \
	memory.cpp \
	root.hpp \
	root.cpp \
	xlsx.hpp \
	xlsx.cpp \
	xls_xml.hpp \
	xls_xml.cpp \
	ods.hpp \
	ods.cpp \
	csv.hpp \
	csv.cpp \
	gnumeric.hpp \
	gnumeric.cpp

_orcus_la_LDFLAGS = -module -avoid-version -export-symbols-regex PyInit__orcus
_orcus_la_CPPFLAGS = -I$(top_srcdir)/include $(AM_CPPFLAGS)
_orcus_la_LIBADD = \
	../liborcus/liborcus-@ORCUS_API_VERSION@.la \
	../parser/liborcus-parser-@ORCUS_API_VERSION@.la \
	$(PYTHON_LIBS)

if BUILD_SPREADSHEET_MODEL

_orcus_la_SOURCES += \
	document.hpp \
	document.cpp \
	sheet.hpp \
	sheet.cpp \
	sheet_rows.hpp \
	sheet_rows.cpp \
	cell.hpp \
	cell.cpp \
	named_expression.hpp \
	named_expression.cpp

_orcus_la_LIBADD += \
	../spreadsheet/liborcus-spreadsheet-model-@ORCUS_API_VERSION@.la \
        $(LIBIXION_LIBS)

endif # BUILD_SPREADSHEET_MODEL

_orcus_json_la_SOURCES = \
	json.cpp

_orcus_json_la_LDFLAGS = -module -avoid-version -export-symbols-regex PyInit__orcus_json
_orcus_json_la_CPPFLAGS = -I$(top_srcdir)/include $(AM_CPPFLAGS)
_orcus_json_la_LIBADD = \
	../liborcus/liborcus-@ORCUS_API_VERSION@.la \
	../parser/liborcus-parser-@ORCUS_API_VERSION@.la \
	$(PYTHON_LIBS)

orcusdir = $(pythondir)/orcus
orcus_DATA = \
	./orcus/__init__.py \
	./orcus/csv.py \
	./orcus/gnumeric.py \
	./orcus/json.py \
	./orcus/ods.py \
	./orcus/xls_xml.py \
	./orcus/xlsx.py

EXTRA_DIST = \
	./orcus/__init__.py \
	./orcus/csv.py \
	./orcus/gnumeric.py \
	./orcus/json.py \
	./orcus/ods.py \
	./orcus/xls_xml.py \
	./orcus/xlsx.py

AM_TESTS_ENVIRONMENT = PYTHONPATH=$(top_srcdir)/src/python:.libs$${PYTHONPATH:+:$${PYTHONPATH}}; export PYTHONPATH;

TESTS = \
	../../test/python/json.py \
	../../test/python/module.py \
	../../test/python/csv.py \
	../../test/python/csv_export.py

if WITH_PYTHON_XLSX

TESTS += ../../test/python/xlsx.py
AM_TESTS_ENVIRONMENT += export WITH_PYTHON_XLSX=1;

endif # WITH_PYTHON_XLSX

if WITH_PYTHON_ODS

TESTS += ../../test/python/ods.py
AM_TESTS_ENVIRONMENT += export WITH_PYTHON_ODS=1;

endif # WITH_PYTHON_ODS

if WITH_PYTHON_XLS_XML

TESTS += ../../test/python/xls_xml.py
AM_TESTS_ENVIRONMENT += export WITH_PYTHON_XLS_XML=1;

endif # WITH_PYTHON_XLS_XML

if WITH_PYTHON_GNUMERIC

TESTS += ../../test/python/gnumeric.py
AM_TESTS_ENVIRONMENT += export WITH_PYTHON_GNUMERIC=1;

endif # WITH_PYTHON_GNUMERIC

endif # BUILD_PYTHON
