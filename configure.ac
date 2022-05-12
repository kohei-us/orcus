#                            -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.65])

# ===================
# Version information
# ===================
m4_define([orcus_major_version], [0])
m4_define([orcus_minor_version], [17])
m4_define([orcus_micro_version], [99])
m4_define([orcus_version], [orcus_major_version.orcus_minor_version.orcus_micro_version])

# ===============
# API information
# ===============
m4_define([orcus_major_api_version], [0])
m4_define([orcus_minor_api_version], [18])
m4_define([orcus_api_version], [orcus_major_version.orcus_minor_api_version])

# =============
# Automake init
# =============
AC_INIT([liborcus],[orcus_version])
AC_CONFIG_MACRO_DIR([m4])
m4_pattern_allow([^BOOST_])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE([1.11 foreign dist-bzip2 dist-xz])
AM_SILENT_RULES([yes])
AC_LANG([C++])
AX_CXX_COMPILE_STDCXX_17([noext], [mandatory])
CXXFLAGS="-fvisibility=hidden $CXXFLAGS -Wall -pthread -Wshadow"

# ===========================
# Find required base packages
# ===========================
AC_PROG_CPP
AC_PROG_CXX
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_LIBTOOL
AC_LIBTOOL_WIN32_DLL
PKG_PROG_PKG_CONFIG([0.20])

# =====================
# Find required headers
# =====================
AC_CHECK_HEADERS([stdlib.h sys/time.h unistd.h])

# =============================================================
# Checks for typedefs, structures, and compiler characteristics
# =============================================================
AC_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_MODE_T
AC_TYPE_SIZE_T

AC_CHECK_SIZEOF([void *])
CXXFLAGS="$CXXFLAGS -DSIZEOF_VOID_P=$ac_cv_sizeof_void_p"

# ============================
# Checks for library functions
# ============================
AC_FUNC_STRTOD
AC_CHECK_FUNCS([gettimeofday])
AC_DEFINE([_REENTRANT], [], [Enable reentrant.])
AC_DEFINE([BOOST_ALL_NO_LIB], [], [Disable boost's evil autolink.])

# ======================
# Set required ixion api
# ======================
IXION_REQUIRED_API_VERSION=0.18
AC_SUBST(IXION_REQUIRED_API_VERSION)

# =============
# Set orcus api
# =============
ORCUS_API_VERSION=orcus_api_version
ORCUS_MAJOR_VERSION=orcus_major_version
ORCUS_MINOR_VERSION=orcus_minor_version
ORCUS_MICRO_VERSION=orcus_micro_version
AC_SUBST(ORCUS_API_VERSION)
AC_SUBST(ORCUS_MAJOR_VERSION)
AC_SUBST(ORCUS_MINOR_VERSION)
AC_SUBST(ORCUS_MICRO_VERSION)

BOOST_REQUIRE([1.36])
BOOST_FILESYSTEM
BOOST_SYSTEM

# =========================================================
# Determine if we are going to build static lib (for MinGW)
# =========================================================
AM_CONDITIONAL([HAVE_STATIC_LIB],
               [test "$enable_shared" = no -a "$enable_static" = yes])

# =====================
# Development Utilities
# =====================
AC_ARG_ENABLE(debug-utils,
        AC_HELP_STRING([--enable-debug-utils], [Build with extra checks useful during development.]),
        [enable_debug_utils="$enableval"],
        [enable_debug_utils=no]
)

# =====
# Debug
# =====
AC_ARG_ENABLE(debug,
        AS_HELP_STRING([--enable-debug], [Build with debug features in mind.]),
        [enable_debug="$enableval"],
        [enable_debug=no]
)
AS_IF([test "x$enable_debug" != "xno"], [
        CXXFLAGS="$CXXFLAGS -g -O0 -DMDDS_ASSERT_STRING_MAP"
        enable_debug_utils=yes
], [
        CXXFLAGS="$CXXFLAGS -O2"
        AC_DEFINE([NDEBUG], [], [Disable debugging information])
])

AS_IF([test "x$enable_debug_utils" != "xno"], [
        CXXFLAGS="$CXXFLAGS -D__ORCUS_DEBUG_UTILS"
])

# zlib is a hard requirement in liborcus-parser.
PKG_CHECK_MODULES([ZLIB], [zlib])

# ==============
# tools (binary)
# ==============
AC_ARG_WITH(tools,
            AS_HELP_STRING([--without-tools],
                           [Disables building of binary executable tools.]),
        [with_tools="$withval"],
        [with_tools=yes]
)

# ==================
# ods filter support
# ==================
AC_ARG_WITH(ods-filter,
            AS_HELP_STRING([--without-ods-filter],
                           [Disables the OpenDocument Format spreadsheet import filter.]),
        [with_ods_filter="$withval"],
        [with_ods_filter=yes]
)

# ===================
# xlsx filter support
# ===================
AC_ARG_WITH(xlsx-filter,
            AS_HELP_STRING([--without-xlsx-filter],
                           [Disables the Microsoft Excel OOXML import filter.]),
        [with_xlsx_filter="$withval"],
        [with_xlsx_filter=yes]
)

# ======================
# xls xml filter support
# ======================
AC_ARG_WITH(xls-xml-filter,
            AS_HELP_STRING([--without-xls-xml-filter],
                           [Disables the Microsoft Excel XML import filter.]),
        [with_xls_xml_filter="$withval"],
        [with_xls_xml_filter=yes]
)

# =======================
# gnumeric filter support
# =======================
AC_ARG_WITH(gnumeric-filter,
            AS_HELP_STRING([--without-gnumeric-filter],
                           [Disables the gnumeric import filter. The gnumeric import filter depends on zLib.]),
        [with_gnumeric_filter="$withval"],
        [with_gnumeric_filter=yes]
)

AC_ARG_WITH(cpu-features,
        AS_HELP_STRING([--with-cpu-features], [Enables custom CPU features.]),
        [with_cpu_features="$withval"],
        [with_cpu_features=no]
)

AM_CONDITIONAL([WITH_ODS_FILTER], [test "x$with_ods_filter" != "xno"])
AM_CONDITIONAL([WITH_XLSX_FILTER], [test "x$with_xlsx_filter" != "xno"])
AM_CONDITIONAL([WITH_XLS_XML_FILTER], [test "x$with_xls_xml_filter" != "xno"])
AM_CONDITIONAL([WITH_GNUMERIC_FILTER], [test "x$with_gnumeric_filter" != "xno"])
AM_CONDITIONAL([WITH_TOOLS], [test "x$with_tools" != "xno"])

AS_IF([test "x$with_ods_filter" != "xno"], [
        CXXFLAGS="$CXXFLAGS -D__ORCUS_ODS"
])

AS_IF([test "x$with_xlsx_filter" != "xno"], [
        CXXFLAGS="$CXXFLAGS -D__ORCUS_XLSX"
])

AS_IF([test "x$with_xls_xml_filter" != "xno"], [
        CXXFLAGS="$CXXFLAGS -D__ORCUS_XLS_XML"
])

AS_IF([test "x$with_gnumeric_filter" != "xno"], [
        CXXFLAGS="$CXXFLAGS -D__ORCUS_GNUMERIC"
        BOOST_IOSTREAMS
])

AS_IF([test "x$with_cpu_features" != "xno"], [
        CXXFLAGS="$CXXFLAGS -D__ORCUS_CPU_FEATURES"
])

AS_IF([test "x$with_tools" != "xno"], [
        BOOST_PROGRAM_OPTIONS
        BOOST_SYSTEM
])

# ============
# mdds support
# ============
PKG_CHECK_MODULES([MDDS],[mdds-2.0 >= 1.9.99])
CXXFLAGS="$CXXFLAGS $MDDS_CFLAGS"

# =================
# Spreadsheet model
# =================
AC_ARG_ENABLE(spreadsheet-model,
        AS_HELP_STRING([--disable-spreadsheet-model],
                [Disable the spreadsheet model implementation in orcus.  Note that the spreadsheet-specific command line utilities will not be built when this is disabled.]),
        [enable_spreadsheet_model="$enableval"],
        [enable_spreadsheet_model=yes]
)
AS_IF([test "x$enable_spreadsheet_model" != "xno"], [
        PKG_CHECK_MODULES([LIBIXION],[libixion-$IXION_REQUIRED_API_VERSION >= 0.16.99])
        CXXFLAGS="$CXXFLAGS -D__ORCUS_SPREADSHEET_MODEL"
        BOOST_DATE_TIME
])
AM_CONDITIONAL([BUILD_SPREADSHEET_MODEL], [test "x$enable_spreadsheet_model" != "xno"])

# ==============
# Python support
# ==============
AC_ARG_ENABLE([python],
    [AS_HELP_STRING([--disable-python], [Disable python bindings])],
    [enable_python="$enableval"],
    [enable_python=yes]
)

# Check for python.
AS_IF([test "x$enable_python" != "xno"], [
    AM_PATH_PYTHON(3)
    PKG_CHECK_MODULES([PYTHON], [python3 >= 0.28])
])
AM_CONDITIONAL([BUILD_PYTHON], [test "x$enable_python" != "xno"])

# csv is always enabled.
CXXFLAGS="$CXXFLAGS -D__ORCUS_PYTHON_CSV"

with_python_xlsx="no"
if test "x$enable_spreadsheet_model" != "xno"; then
    if test "x$enable_python" != "xno"; then
        if test "x$with_xlsx_filter" != "xno"; then
            with_python_xlsx="yes"
        fi
    fi
fi

if test "x$with_python_xlsx" != "xno"; then
    CXXFLAGS="$CXXFLAGS -D__ORCUS_PYTHON_XLSX"
fi

with_python_ods="no"
if test "x$enable_spreadsheet_model" != "xno"; then
    if test "x$enable_python" != "xno"; then
        if test "x$with_ods_filter" != "xno"; then
            with_python_ods="yes"
        fi
    fi
fi

if test "x$with_python_ods" != "xno"; then
    CXXFLAGS="$CXXFLAGS -D__ORCUS_PYTHON_ODS"
fi

with_python_xls_xml="no"
if test "x$enable_spreadsheet_model" != "xno"; then
    if test "x$enable_python" != "xno"; then
        if test "x$with_xls_xml_filter" != "xno"; then
            with_python_xls_xml="yes"
        fi
    fi
fi

if test "x$with_python_xls_xml" != "xno"; then
    CXXFLAGS="$CXXFLAGS -D__ORCUS_PYTHON_XLS_XML"
fi

with_python_gnumeric="no"
if test "x$enable_spreadsheet_model" != "xno"; then
    if test "x$enable_python" != "xno"; then
        if test "x$with_gnumeric_filter" != "xno"; then
            with_python_gnumeric="yes"
        fi
    fi
fi

if test "x$with_python_gnumeric" != "xno"; then
    CXXFLAGS="$CXXFLAGS -D__ORCUS_PYTHON_GNUMERIC"
fi

AM_CONDITIONAL([WITH_PYTHON_XLSX], [test "x$with_python_xlsx" != "xno"])
AM_CONDITIONAL([WITH_PYTHON_ODS], [test "x$with_python_ods" != "xno"])
AM_CONDITIONAL([WITH_PYTHON_CSV], [test "x$with_python_csv" != "xno"])
AM_CONDITIONAL([WITH_PYTHON_XLS_XML], [test "x$with_python_xls_xml" != "xno"])
AM_CONDITIONAL([WITH_PYTHON_GNUMERIC], [test "x$with_python_gnumeric" != "xno"])

# ==============
# Werror support
# ==============

# NB: This must come at the end of all the other header detections, as enabling
# it may cause some header detections to fail.

AC_ARG_ENABLE([werror],
        [AS_HELP_STRING([--enable-werror], [Treat all warnings as errors, useful for development])],
        [enable_werror="$enableval"],
        [enable_werror=no]
)
AS_IF([test x"$enable_werror" = "xyes"], [
        CXXFLAGS="$CXXFLAGS -Werror"
])

AX_VALGRIND_CHECK()

AC_CONFIG_FILES([Makefile
        liborcus-$ORCUS_API_VERSION.pc:liborcus.pc.in
        liborcus-spreadsheet-model-$ORCUS_API_VERSION.pc:liborcus-spreadsheet-model.pc.in
        include/Makefile
        include/orcus/Makefile
        include/orcus/detail/Makefile
        include/orcus/spreadsheet/Makefile
        src/Makefile
        src/include/Makefile
        src/include/mso/Makefile
        src/liborcus/Makefile
        src/liborcus/constants.inl
        src/mso/Makefile
        src/parser/Makefile
        src/python/Makefile
        src/spreadsheet/Makefile
        src/test/Makefile
        test/python/env.json
        benchmark/Makefile
        doc_example/Makefile
])
AC_OUTPUT

# ==============================================
# Display final informations about configuration
# ==============================================
AC_MSG_NOTICE([
==============================================================================
Build configuration:
        debug                  $enable_debug
        debug-utils            $enable_debug_utils
        werror                 $enable_werror
        spreadsheet-model      $enable_spreadsheet_model
        gnumeric-filter        $with_gnumeric_filter
        ods-filter             $with_ods_filter
        xlsx-filter            $with_xlsx_filter
        xls-xml-filter         $with_xls_xml_filter
        xls-gnumeric           $with_gnumeric_filter
        tools                  $with_tools
        python                 $enable_python
        python-xlsx            $with_python_xlsx
        python-ods             $with_python_ods
        python-xls-xml         $with_python_xls_xml
        python-gnumeric        $with_python_gnumeric
        cpu-features           $with_cpu_features
==============================================================================
])

