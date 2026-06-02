#!/usr/bin/env python3
########################################################################
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
########################################################################

import gc
import sys
import pytest
from pathlib import Path

from orcus import xlsx

import file_load_common as common


TESTDIR = (Path(__file__).parent / ".." / "xlsx").resolve()


@pytest.mark.parametrize("test_dir_name", [
    "boolean-values",
    "empty-shared-strings",
    "formula-array-1",
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
    common.run_test_dir(TESTDIR / test_dir_name, xlsx)


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


def test_sheets_refcount_balance():
    # The document's sheets tuple must own exactly one reference per sheet.
    filepath = TESTDIR / "raw-values-1" / "input.xlsx"
    with filepath.open("rb") as f:
        doc = xlsx.read(f)

    # The tuple itself should hold exactly one reference to each sheet.
    # doc.sheets[0] places one transient reference on the evaluation stack;
    # getrefcount sees that plus the tuple's owned reference.
    refs_via_tuple_only = sys.getrefcount(doc.sheets[0])
    assert refs_via_tuple_only == 2, (
        f"sheet refcount is {refs_via_tuple_only}; expected 2 "
        f"(1 owned by the tuple, 1 transient on the eval stack); a "
        f"larger value indicates store_document leaks a reference")

    sheet = doc.sheets[0]
    before = sys.getrefcount(sheet)

    del doc
    gc.collect()

    after = sys.getrefcount(sheet)

    assert before - after == 1, (
        f"refcount on sheet dropped by {before - after} after destroying "
        f"doc; expected exactly 1 (the tuple's owned reference)")


def test_row_cells_refcount_balance():
    # Each row tuple must own exactly one reference per cell object it holds.
    filepath = TESTDIR / "raw-values-1" / "input.xlsx"
    with filepath.open("rb") as f:
        doc = xlsx.read(f)

    sheet = doc.sheets[0]

    # rows come from an iterator (SheetRows tp_iter/tp_iternext); each
    # iteration yields a fresh tuple of cell objects.
    first_row = next(iter(sheet.get_rows()))

    # The row tuple should hold exactly one reference to each cell.
    # first_row[0] places one transient reference on the evaluation stack;
    # getrefcount sees that plus the tuple's owned reference.
    refs_via_tuple_only = sys.getrefcount(first_row[0])
    assert refs_via_tuple_only == 2, (
        f"cell refcount is {refs_via_tuple_only}; expected 2 "
        f"(1 owned by the row tuple, 1 transient on the eval stack); a "
        f"larger value indicates the row builder leaks a reference")

    # Use a multi-cell row so a per-slot leak on column N>0 can't hide behind
    # a balanced column 0.
    row = next(
        r for r in sheet.get_rows()
        if sum(1 for c in r if str(c.type) != "CellType.EMPTY") >= 2)
    cells = list(row)

    before = [sys.getrefcount(c) for c in cells]
    del row
    gc.collect()
    after = [sys.getrefcount(c) for c in cells]

    for col, (b, a) in enumerate(zip(before, after)):
        assert b - a == 1, (
            f"refcount on cell at column {col} dropped by {b - a} after "
            f"destroying the row; expected exactly 1 (the row tuple's owned "
            f"reference)")


def test_document_refcount_balance():
    # xlsx.read returns a new-reference document object built by the document
    # factory; the caller must receive exactly one owned reference.
    filepath = TESTDIR / "raw-values-1" / "input.xlsx"
    with filepath.open("rb") as f:
        doc = xlsx.read(f)

    # the 'doc' binding owns the only reference; getrefcount also sees the
    # transient reference on the evaluation stack
    refs = sys.getrefcount(doc)
    assert refs == 2, (
        f"document refcount is {refs}; expected 2 (1 owned by the caller, "
        f"1 transient on the eval stack); a larger value indicates the "
        f"document factory leaks a reference")


def test_named_expressions_refcount_balance():
    # get_named_expressions() returns a fresh new-reference object built via
    # create_object_from_type; the factory must hand back exactly one
    # owned reference.
    filepath = TESTDIR / "named-expression" / "input.xlsx"
    with filepath.open("rb") as f:
        doc = xlsx.read(f)

    named_exps = doc.get_named_expressions()
    refs = sys.getrefcount(named_exps)
    assert refs == 2, (
        f"named expressions object refcount is {refs}; expected 2 (1 owned "
        f"by the caller, 1 transient on the eval stack); a larger value "
        f"indicates the factory leaks a reference")

    # repeatedly creating and dropping the object must not leak a reference to
    # the document it borrows from
    before = sys.getrefcount(doc)
    for _ in range(100):
        doc.get_named_expressions()
    gc.collect()
    after = sys.getrefcount(doc)
    assert before == after, (
        f"document refcount changed from {before} to {after} after creating "
        f"transient named-expression objects; the factory leaks a reference")


if __name__ == "__main__":
    sys.exit(pytest.main([__file__]))
