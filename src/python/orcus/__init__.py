#######################################################################
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
########################################################################

try:
    from _orcus import *
    from _orcus import __version__
except ModuleNotFoundError:
    # We do this to enable sphinx to generate documentation without having to
    # build the C++ part.
    pass

from enum import Enum


class FormatType(Enum):
    """Collection of file format types currently used in orcus."""

    UNKNOWN    = 0
    ODS        = 1
    XLSX       = 2
    GNUMERIC   = 3
    XLS_XML    = 4
    CSV        = 5
    YAML       = 6
    JSON       = 7
    XML        = 8


class CellType(Enum):
    """Collection of cell types stored in spreadsheet."""

    UNKNOWN = 0
    EMPTY   = 1
    BOOLEAN = 2
    NUMERIC = 3
    STRING  = 4
    STRING_WITH_ERROR = 5
    FORMULA = 6
    FORMULA_WITH_ERROR = 7


class FormulaTokenType(Enum):
    """Collection of formula token types."""

    UNKNOWN   = 0
    REFERENCE = 1
    VALUE     = 2
    NAME      = 3
    FUNCTION  = 4
    OPERATOR  = 5
    ERROR     = 6


class FormulaTokenOp(Enum):
    """Collection of formula token operators."""

    UNKNOWN          = 0
    SINGLE_REF       = 1
    RANGE_REF        = 2
    TABLE_REF        = 3
    NAMED_EXPRESSION = 4
    STRING           = 5
    VALUE            = 6
    FUNCTION         = 7
    PLUS             = 8
    MINUS            = 9
    DIVIDE           = 10
    MULTIPLY         = 11
    EXPONENT         = 12
    CONCAT           = 13
    EQUAL            = 14
    NOT_EQUAL        = 15
    LESS             = 16
    GREATER          = 17
    LESS_EQUAL       = 18
    GREATER_EQUAL    = 19
    OPEN             = 20
    CLOSE            = 21
    SEP              = 22
    ERROR            = 23


def get_document_loader_module(format_type):
    """Obtain a document loader module for the specified format type.

    Args:
        format_type (orcus.FormatType):
            Format type for which to load a document loader.

    Returns:
        Document loader module.
    """
    m = None
    if format_type == FormatType.ODS:
        from . import ods as m
    elif format_type == FormatType.XLSX:
        from . import xlsx as m
    elif format_type == FormatType.XLS_XML:
        from . import xls_xml as m
    elif format_type == FormatType.GNUMERIC:
        from . import gnumeric as m
    elif format_type == FormatType.CSV:
        from . import csv as m

    return m
