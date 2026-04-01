#!/usr/bin/env python3
########################################################################
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
########################################################################

import logging
import pytest
import sys
from pathlib import Path

import orcus
from orcus import csv


TESTDIR = Path(__file__).parent / ".." / "csv" / "text-qualifiers"

logger = logging.getLogger(__name__)


# These test files all have the same content but with different text qualifiers
@pytest.mark.parametrize("filename, text_qualifier", [
    ("backtick.csv", "`"),
    ("pipe.csv", "|"),
    ("single-quote.csv", "'"),
])
def test_text_qualifier(filename, text_qualifier):
    input_path = TESTDIR / filename
    logger.info(f"input file: {input_path}")

    doc = csv.read(input_path.open("r"), text_qualifier=text_qualifier)
    assert len(doc.sheets) == 1
    rows = [row for row in doc.sheets[0].get_rows()]
    assert len(rows) == 4
    row = rows[0]
    assert row[0].value == "Field 1"
    assert row[1].value == "Field 2"
    assert row[2].value == "Field 3"
    row = rows[1]
    assert row[0].value == "one"
    assert row[1].value == "two"
    assert row[2].value == "three"
    row = rows[2]
    assert row[0].value == "with, comma"
    assert row[1].value == "unquoted"
    assert row[2].value == "also unquoted"
    row = rows[3]
    assert row[0].value == "has spaces inside"
    assert row[1].value == 123
    assert row[2].value == "end"


def test_invalid_qualifier():
    input_path = TESTDIR / "backtick.csv"

    with pytest.raises(ValueError):
        doc = csv.read(input_path.open("r"), text_qualifier="``")  # 2 chars

    with pytest.raises(ValueError):
        doc = csv.read(input_path.open("r"), text_qualifier="")  # empty qualifier


if __name__ == "__main__":
    sys.exit(pytest.main([__file__]))

