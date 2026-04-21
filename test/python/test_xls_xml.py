#!/usr/bin/env python3
########################################################################
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
########################################################################

import sys
import pytest
from pathlib import Path

import orcus
from orcus import xls_xml

import file_load_common as common


TESTDIR = (Path(__file__).parent / ".." / "xls-xml").resolve()


@pytest.mark.parametrize("test_dir_name", [
    "basic",
    "basic-utf-16-be",
    "basic-utf-16-le",
    "bold-and-italic",
    "colored-text",
    "empty-rows",
    "formula-array-1",
    "formula-cells-1",
    "formula-cells-2",
    "formula-cells-3",
    "invalid-sub-structure",
    "leading-whitespace",
    "linebreak",
    "merged-cells",
    "named-colors",
    "named-expression",
    "named-expression-sheet-local",
    "raw-values-1",
    "table-offset",
    "unnamed-parent-styles",
])
def test_import(test_dir_name):
    common.run_test_dir(TESTDIR / test_dir_name, xls_xml)


def test_skip_error_cells():
    filepath = TESTDIR / "formula-cells-parse-error" / "input.xml"
    data = filepath.read_bytes()

    with pytest.raises(RuntimeError):
        xls_xml.read(data)

    with pytest.raises(RuntimeError):  # TODO : should we raise a more specific error?
        xls_xml.read(data, error_policy="fail")

    # With the 'skip' policy, formula cells with erroneous formulas are
    # imported as formula cells with error.
    doc = xls_xml.read(data, error_policy="skip")

    # Make sure cells B2 and A5 are imported as formula cells.
    rows = [row for row in doc.sheets[0].get_rows()]
    c = rows[1][1]
    assert c.type == orcus.CellType.FORMULA_WITH_ERROR
    assert not c.formula  # formula string should be empty
    # error formula tokens consist of: error token, string token (original formula), string token (error message).
    formula_tokens = [t for t in c.get_formula_tokens()]
    assert formula_tokens[0].type == orcus.FormulaTokenType.ERROR
    assert formula_tokens[1].type == orcus.FormulaTokenType.VALUE
    assert formula_tokens[2].type == orcus.FormulaTokenType.VALUE
    c = rows[4][0]
    assert c.type == orcus.CellType.FORMULA_WITH_ERROR
    assert not c.formula  # formula string should be empty
    formula_tokens = [t for t in c.get_formula_tokens()]
    assert formula_tokens[0].type == orcus.FormulaTokenType.ERROR
    assert formula_tokens[1].type == orcus.FormulaTokenType.VALUE
    assert formula_tokens[2].type == orcus.FormulaTokenType.VALUE


if __name__ == "__main__":
    sys.exit(pytest.main([__file__]))
