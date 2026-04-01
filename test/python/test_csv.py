#!/usr/bin/env python3
########################################################################
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
########################################################################

import pytest
import sys
from pathlib import Path

import orcus
from orcus import csv

import file_load_common as common


TESTDIR = Path(__file__).parent / ".." / "csv"


@pytest.mark.parametrize("test_dir_name", [
    "simple-numbers",
    "normal-quotes",
    "double-quotes",
    "quoted-with-delim",
])
def test_import(test_dir_name):
    test_dir = TESTDIR / test_dir_name
    print(f"test directory: {test_dir}")

    expected = common.ExpectedDocument(test_dir / "check.txt")
    input_file = test_dir / "input.csv"
    print(f"input file: {input_file}")

    doc = csv.read(input_file.open("r"))

    assert isinstance(doc, orcus.Document)
    assert len(expected.sheets) > 0
    assert len(expected.sheets) <= len(doc.sheets)

    expected_sheets = {sh.name: sh for sh in expected.sheets}

    for actual_sheet in doc.sheets:
        assert actual_sheet.name in expected_sheets
        expected_sheet = expected_sheets[actual_sheet.name]
        assert expected_sheet.data_size == actual_sheet.data_size
        for row, (expected_row, actual_row) in enumerate(zip(expected_sheet.get_rows(), actual_sheet.get_rows())):
            for col, (expected_cell, actual_cell) in enumerate(zip(expected_row, actual_row)):
                result, err = common.compare_cells(expected_cell, actual_cell)
                assert result, f"unexpected cell value (row={row}; col={col}; error='{err}')"

    # Also verify loading from in-memory string value.
    content = input_file.read_text()
    doc = csv.read(content)
    assert isinstance(doc, orcus.Document)


if __name__ == "__main__":
    sys.exit(pytest.main([__file__]))
