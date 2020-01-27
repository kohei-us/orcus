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

from orcus import xlsx

import file_load_common as common


class TestCase(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        # base directory for xlsx test files.
        basedir = os.path.join(os.path.dirname(__file__), "..", "xlsx")
        cls.basedir = os.path.normpath(basedir)

    def test_import(self):

        test_dirs = (
            "boolean-values",
            "empty-shared-strings",
            "formula-cells",
            "formula-shared",
            "named-expression",
            "named-expression-sheet-local",
            "raw-values-1",
        )

        for test_dir in test_dirs:
            test_dir = os.path.join(self.basedir, test_dir)
            common.run_test_dir(self, test_dir, xlsx)

    def test_named_expression(self):
        filepath = os.path.join(self.basedir, "named-expression", "input.xlsx")
        with open(filepath, "rb") as f:
            doc = xlsx.read(f.read())
        print(doc)
        print(doc.named_expressions)


if __name__ == '__main__':
    unittest.main()
