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

import orcus
from orcus import xls_xml

import file_load_common as common


class TestCase(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        # base directory for ods test files.
        basedir = os.path.join(os.path.dirname(__file__), "..", "xls-xml")
        cls.basedir = os.path.normpath(basedir)

    def test_import(self):

        test_dirs = (
            "basic",
            "bold-and-italic",
            "colored-text",
            "empty-rows",
            "merged-cells",
            "named-expression",
            "named-expression-sheet-local",
            "raw-values-1",
        )

        for test_dir in test_dirs:
            test_dir = os.path.join(self.basedir, test_dir)
            common.run_test_dir(self, test_dir, xls_xml)

    def test_skip_error_cells(self):
        filepath = os.path.join(self.basedir, "formula-cells-parse-error", "input.xml")
        with open(filepath, "rb") as f:
            bytes = f.read()

        with self.assertRaises(RuntimeError):
            doc = xls_xml.read(bytes)

        with self.assertRaises(RuntimeError):  # TODO : should we raise a more specific error?
            doc = xls_xml.read(bytes, error_policy="fail")

        # With the 'skip' policy, formula cells with erroneous formulas are
        # imported as formula cells with error.
        doc = xls_xml.read(bytes, error_policy="skip")

        # Make sure cells B2 and A5 are imported as formula cells.
        rows = [row for row in doc.sheets[0].get_rows()]
        c = rows[1][1]
        self.assertEqual(c.type, orcus.CellType.FORMULA)
        self.assertFalse(c.formula)  # formula string should be empty
        # error formula tokens consist of: error token, string token (original formula), string token (error message).
        self.assertEqual(c.formula_tokens[0].type, orcus.FormulaTokenType.ERROR)
        self.assertEqual(c.formula_tokens[1].type, orcus.FormulaTokenType.VALUE)
        self.assertEqual(c.formula_tokens[2].type, orcus.FormulaTokenType.VALUE)
        c = rows[4][0]
        self.assertEqual(c.type, orcus.CellType.FORMULA)
        self.assertFalse(c.formula)  # formula string should be empty
        self.assertEqual(c.formula_tokens[0].type, orcus.FormulaTokenType.ERROR)
        self.assertEqual(c.formula_tokens[1].type, orcus.FormulaTokenType.VALUE)
        self.assertEqual(c.formula_tokens[2].type, orcus.FormulaTokenType.VALUE)


if __name__ == '__main__':
    unittest.main()
