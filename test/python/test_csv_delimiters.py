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


TESTDIR = Path(__file__).parent / ".." / "csv" / "delimiters"


def test_space_delimiter():
    input_path = TESTDIR / "space.csv"

    doc = csv.read(input_path.open("r"), delimiters=" ")
    assert len(doc.sheets) == 1
    rows = [row for row in doc.sheets[0].get_rows()]
    assert len(rows) == 3
    row = rows[0]
    assert row[0].value == "F1"
    assert row[1].value == "F2"
    assert row[2].value == "F3"
    row = rows[1]
    assert row[0].value == 1
    assert row[1].value == 2
    assert row[2].value == 3
    row = rows[2]
    assert row[0].value == 4
    assert row[1].value == 5
    assert row[2].value == 6


def test_tab_delimiter():
    input_path = TESTDIR / "tab.csv"

    doc = csv.read(input_path.open("r"), delimiters="\t")
    assert len(doc.sheets) == 1
    rows = [row for row in doc.sheets[0].get_rows()]
    assert len(rows) == 3
    row = rows[0]
    assert row[0].value == "F1"
    assert row[1].value == "F2"
    assert row[2].value == "F3"
    row = rows[1]
    assert row[0].value == 1
    assert row[1].value == 2
    assert row[2].value == 3
    row = rows[2]
    assert row[0].value == 4
    assert row[1].value == 5
    assert row[2].value == 6


if __name__ == "__main__":
    sys.exit(pytest.main([__file__]))

