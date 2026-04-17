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


if __name__ == "__main__":
    sys.exit(pytest.main([__file__]))
