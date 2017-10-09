#!/usr/bin/env python3
########################################################################
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
########################################################################

import unittest
import os
import os.path
import file_load_common as common
from orcus import FormatType, csv


class MockFileObject(object):

    def __init__(self):
        self.__bytes = None

    def write(self, bytes):
        self.__bytes = bytes

    def read(self):
        return self.__bytes

    @property
    def bytes(self):
        return self.__bytes


class TestCase(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        # base directory for xlsx test files.
        basedir_xlsx = os.path.join(os.path.dirname(__file__), "..", "xlsx")
        cls.basedir_xlsx = os.path.normpath(basedir_xlsx)

    @unittest.skipIf(os.environ.get("WITH_PYTHON_XLSX") is None, "python xlsx module is disabled")
    def test_export_from_xlsx(self):
        from orcus import xlsx

        test_dirs = (
            "raw-values-1",
            "empty-shared-strings",
            "named-expression",
        )

        for test_dir in test_dirs:
            test_dir = os.path.join(self.basedir_xlsx, test_dir)
            input_file = os.path.join(test_dir, "input.xlsx")
            with open(input_file, "rb") as f:
                doc = xlsx.read(f)

            # Build an expected document object from the check file.
            check_file = os.path.join(test_dir, "check.txt")
            check_doc = common.ExpectedDocument(check_file)

            # check_doc only contains non-empty sheets.
            data_sheet_names = set()
            for sheet in check_doc.sheets:
                data_sheet_names.add(sheet.name)

            for sheet in doc.sheets:
                mfo = MockFileObject()
                sheet.write(mfo, format=FormatType.CSV)

                if mfo.bytes is None:
                    self.assertFalse(sheet.name in data_sheet_names)
                    continue

                # Load the csv stream into a document again.
                doc_reload = csv.read(mfo)
                self.assertEqual(1, len(doc_reload.sheets))
                for row1, row2 in zip(sheet.get_rows(), doc_reload.sheets[0].get_rows()):
                    self.assertEqual(row1, row2)

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
                with self.assertRaises(Exception):
                    doc.sheets[0].write(mfo, format=invalid_format)


if __name__ == '__main__':
    unittest.main()


