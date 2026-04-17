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

from orcus import xlsx

import file_load_common as common


TESTDIR = (Path(__file__).parent / ".." / "xlsx").resolve()


@pytest.mark.parametrize("test_dir_name", [
    "boolean-values",
    "empty-shared-strings",
    "formula-cells",
    "formula-shared",
    "formula-with-string-results",
    "linebreak",
    "named-expression",
    "named-expression-sheet-local",
    "raw-values-1",
    "raw-values-2",
])
def test_import(test_dir_name):
    common.run_test_dir(TESTDIR / test_dir_name, common.DocLoader(xlsx))


def test_named_expression():
    filepath = TESTDIR / "named-expression" / "input.xlsx"
    doc = xlsx.read(filepath.open("rb"))

    named_exps = doc.get_named_expressions()
    assert named_exps.names() == {"MyRange", "MyRange2"}
    assert len(named_exps) == 2

    named_exps_dict = {x[0]: x[1] for x in named_exps}
    exp = named_exps_dict["MyRange"]
    assert exp.origin == "Sheet1!$A$1"
    assert exp.formula == "$A$1:$A$5"
    iter = exp.get_formula_tokens()
    assert len(iter) == 1
    tokens = [t for t in iter]
    assert str(tokens[0]) == "$A$1:$A$5"

    exp = named_exps_dict["MyRange2"]
    assert exp.origin == "Sheet1!$A$1"
    assert exp.formula == "$A$1:$B$5"
    iter = exp.get_formula_tokens()
    assert len(iter) == 1
    tokens = [t for t in iter]
    assert str(tokens[0]) == "$A$1:$B$5"


def test_named_expression_sheet_local():
    filepath = TESTDIR / "named-expression-sheet-local" / "input.xlsx"
    doc = xlsx.read(filepath.open("rb"))

    sheet = doc.sheets[0]
    named_exps = sheet.get_named_expressions()
    assert len(named_exps) == 1
    assert named_exps.names() == {"MyRange"}

    named_exps_dict = {x[0]: x[1] for x in named_exps}
    exp = named_exps_dict["MyRange"]
    assert exp.formula == "$A$1:$B$3"
    iter = exp.get_formula_tokens()
    assert len(iter) == 1
    tokens = [t for t in iter]
    assert str(tokens[0]) == "$A$1:$B$3"

    sheet = doc.sheets[1]
    named_exps = sheet.get_named_expressions()
    assert named_exps.names() == {"MyRange"}
    assert len(named_exps) == 1

    named_exps_dict = {x[0]: x[1] for x in named_exps}
    exp = named_exps_dict["MyRange"]
    assert exp.formula == "$A$4:$B$5"
    iter = exp.get_formula_tokens()
    assert len(iter) == 1
    tokens = [t for t in iter]
    assert str(tokens[0]) == "$A$4:$B$5"


if __name__ == "__main__":
    sys.exit(pytest.main([__file__]))
