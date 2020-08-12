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
	formula_token.hpp \
	formula_token.cpp \
	formula_tokens.hpp \
	formula_tokens.cpp \
	named_expressions.hpp \
	named_expressions.cpp \
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
orcustoolsdir = $(pythondir)/orcus/tools

orcus_DATA = \
	./orcus/__init__.py \
	./orcus/csv.py \
	./orcus/gnumeric.py \
	./orcus/json.py \
	./orcus/ods.py \
	./orcus/xls_xml.py \
	./orcus/xlsx.py

orcustools_DATA = \
	./orcus/tools/__init__.py \
	./orcus/tools/bugzilla.py \
	./orcus/tools/file_processor.py

EXTRA_DIST = \
	./orcus/__init__.py \
	./orcus/csv.py \
	./orcus/gnumeric.py \
	./orcus/json.py \
	./orcus/ods.py \
	./orcus/xls_xml.py \
	./orcus/xlsx.py \
	./orcus/tools/__init__.py \
	./orcus/tools/bugzilla.py \
	./orcus/tools/file_processor.py

AM_TESTS_ENVIRONMENT = PYTHONPATH=$(top_srcdir)/src/python:.libs$${PYTHONPATH:+:$${PYTHONPATH}}; export PYTHONPATH;

TESTS = \
	../../test/python/test_json.py \
	../../test/python/test_module.py \
	../../test/python/test_csv.py \
	../../test/python/test_csv_export.py

if WITH_PYTHON_XLSX

TESTS += ../../test/python/test_xlsx.py
AM_TESTS_ENVIRONMENT += export WITH_PYTHON_XLSX=1;

endif # WITH_PYTHON_XLSX

if WITH_PYTHON_ODS

TESTS += ../../test/python/test_ods.py
AM_TESTS_ENVIRONMENT += export WITH_PYTHON_ODS=1;

endif # WITH_PYTHON_ODS

if WITH_PYTHON_XLS_XML

TESTS += ../../test/python/test_xls_xml.py
AM_TESTS_ENVIRONMENT += export WITH_PYTHON_XLS_XML=1;

endif # WITH_PYTHON_XLS_XML

if WITH_PYTHON_GNUMERIC

TESTS += ../../test/python/test_gnumeric.py
AM_TESTS_ENVIRONMENT += export WITH_PYTHON_GNUMERIC=1;

endif # WITH_PYTHON_GNUMERIC

endif # BUILD_PYTHON
