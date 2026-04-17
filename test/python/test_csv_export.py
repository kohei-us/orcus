#!/usr/bin/env python3
########################################################################
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
########################################################################

import os
import sys
import pytest
from pathlib import Path

import file_load_common as common
from orcus import FormatType, csv


TESTDIR_XLSX = (Path(__file__).parent / ".." / "xlsx").resolve()


class MockFileObject(object):

    def __init__(self):
        self._content = None

    def write(self, bytes):
        self._content = bytes

    def read(self):
        return self._content

    @property
    def bytes(self):
        return self._content


@pytest.mark.skipif(os.environ.get("WITH_PYTHON_XLSX") is None, reason="python xlsx module is disabled")
def test_export_from_xlsx():
    from orcus import xlsx

    test_dirs = (
        "raw-values-1",
        "empty-shared-strings",
        "named-expression",
    )

    for test_dir_name in test_dirs:
        test_dir = TESTDIR_XLSX / test_dir_name
        input_file = test_dir / "input.xlsx"
        doc = xlsx.read(input_file.open("rb"))

        # Build an expected document object from the check file.
        check_doc = common.ExpectedDocument(test_dir / "check.txt")

        # check_doc only contains non-empty sheets.
        data_sheet_names = set()
        for sheet in check_doc.sheets:
            data_sheet_names.add(sheet.name)

        for sheet in doc.sheets:
            mfo = MockFileObject()
            sheet.write(mfo, format=FormatType.CSV)

            if mfo.bytes is None:
                assert sheet.name not in data_sheet_names
                continue

            # Load the csv stream into a document again.
            doc_reload = csv.read(mfo)
            assert len(doc_reload.sheets) == 1
            for row1, row2 in zip(sheet.get_rows(), doc_reload.sheets[0].get_rows()):
                # Only compare cell values, not cell types.
                row1 = [c.value for c in row1]
                row2 = [c.value for c in row2]
                assert row1 == row2

        # Make sure we raise an exception on invalid format type.
        # We currently only support exporting sheet as csv.

        invalid_formats = (
            "foo",
            FormatType.GNUMERIC,
            FormatType.JSON,
            FormatType.ODS,
            FormatType.XLSX,
            FormatType.XLS_XML,
            FormatType.XLS_XML,
            FormatType.XML,
            FormatType.YAML,
        )

        for invalid_format in invalid_formats:
            mfo = MockFileObject()
            with pytest.raises(Exception):
                doc.sheets[0].write(mfo, format=invalid_format)


if __name__ == "__main__":
    sys.exit(pytest.main([__file__]))
