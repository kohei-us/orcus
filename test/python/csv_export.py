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

from orcus import FormatType


class MockFileObject(object):

    def __init__(self):
        self.__bytes = None

    def write(self, bytes):
        self.__bytes = bytes

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
            "empty-shared-strings",
        )

        for test_dir in test_dirs:
            test_dir = os.path.join(self.basedir_xlsx, test_dir)
            input_file = os.path.join(test_dir, "input.xlsx")
            with open(input_file, "rb") as f:
                doc = xlsx.read(f)

            for sheet in doc.sheets:
                mfo = MockFileObject()
                sheet.write(mfo, format=FormatType.CSV)
                # TODO : check the contents of the bytes.
                print(mfo.bytes)


if __name__ == '__main__':
    unittest.main()


