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

from orcus import gnumeric

import file_load_common as common


TESTDIR = (Path(__file__).parent / ".." / "gnumeric").resolve()


@pytest.mark.parametrize("test_dir_name", [
    "formula-cells",
    "linebreak",
    "named-expression",
    "named-expression-sheet-local",
    "raw-values-1",
])
def test_import(test_dir_name):
    common.run_test_dir(TESTDIR / test_dir_name, common.DocLoader(gnumeric))


if __name__ == "__main__":
    sys.exit(pytest.main([__file__]))
