#######################################################################
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
########################################################################

from _orcus import *
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
    FORMULA = 5


class FormulaTokenType(Enum):
    """Collection of formula token types."""

    UNKNOWN   = 0
    REFERENCE = 1
    VALUE     = 2
    NAME      = 3
    FUNCTION  = 4
    OPERATOR  = 5
    ERROR     = 6
