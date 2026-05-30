#!/usr/bin/env python3
########################################################################
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
########################################################################

import json
import os
import sys
import pytest
from pathlib import Path

import orcus


TESTDIR = Path(__file__).parent / ".."


@pytest.fixture(scope="module")
def env():
    top_builddir = Path(os.environ["BUILDDIR"])
    return json.load((top_builddir / "test" / "python" / "env.json").open("r"))


def test_version(env):
    s = orcus.__version__
    expected = f"{env['version-major']}.{env['version-minor']}.{env['version-micro']}"
    assert expected == s


@pytest.mark.parametrize("path_parts, expected_format", [
    (("ods", "raw-values-1", "input.ods"), orcus.FormatType.ODS),
    (("xlsx", "raw-values-1", "input.xlsx"), orcus.FormatType.XLSX),
    (("xls-xml", "raw-values-1", "input.xml"), orcus.FormatType.XLS_XML),
    (("gnumeric", "raw-values-1", "input.gnumeric"), orcus.FormatType.GNUMERIC),
])
def test_detect_format(path_parts, expected_format):
    filepath = TESTDIR.joinpath(*path_parts)
    with filepath.open("rb") as f:
        fmt = orcus.detect_format(f)
        assert expected_format == fmt

        f.seek(0)
        data = f.read()
        fmt = orcus.detect_format(data)
        assert expected_format == fmt


class _RaisingStream:
    def read(self):
        raise ValueError("read failure propagated")


def test_detect_format_propagates_read_exception():
    # read_stream_from_args: a read() that raises must surface its own
    # exception rather than a generic "failed to extract bytes" error.
    with pytest.raises(ValueError):
        orcus.detect_format(_RaisingStream())


def test_format_read_propagates_read_exception():
    # read_stream_and_formula_params_from_args, the per-format read path.
    from orcus import ods
    with pytest.raises(ValueError):
        ods.read(_RaisingStream())


def test_cell_type_refcount_balance():
    # Cell(type=...) stored the borrowed reference from PyArg and DECREF'd it
    # in dealloc, leaking a reference off the passed CellType each round.
    ct = orcus.CellType.STRING
    before = sys.getrefcount(ct)
    cell = orcus.Cell(type=ct)
    del cell
    after = sys.getrefcount(ct)
    assert after == before


if __name__ == "__main__":
    sys.exit(pytest.main([__file__]))
