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

        named_exps = doc.get_named_expressions()
        self.assertEqual(named_exps.names(), {"MyRange", "MyRange2"})
        self.assertEqual(len(named_exps), 2)

        named_exps_dict = {x[0]: x[1] for x in named_exps}
        exp = named_exps_dict["MyRange"]
        self.assertEqual(exp.origin, "Sheet1!$A$1")
        self.assertEqual(exp.formula, "$A$1:$A$5")
        iter = exp.get_formula_tokens()
        self.assertEqual(len(iter), 1)
        tokens = [t for t in iter]
        self.assertEqual(str(tokens[0]), "$A$1:$A$5")

        exp = named_exps_dict["MyRange2"]
        self.assertEqual(exp.origin, "Sheet1!$A$1")
        self.assertEqual(exp.formula, "$A$1:$B$5")
        iter = exp.get_formula_tokens()
        self.assertEqual(len(iter), 1)
        tokens = [t for t in iter]
        self.assertEqual(str(tokens[0]), "$A$1:$B$5")

    def test_named_expression_sheet_local(self):
        filepath = os.path.join(self.basedir, "named-expression-sheet-local", "input.xlsx")
        with open(filepath, "rb") as f:
            doc = xlsx.read(f.read())

        sheet = doc.sheets[0]
        named_exps = sheet.get_named_expressions()
        self.assertEqual(len(named_exps), 1)
        self.assertEqual(named_exps.names(), {"MyRange",})

        named_exps_dict = {x[0]: x[1] for x in named_exps}
        exp = named_exps_dict["MyRange"]
        self.assertEqual(exp.formula, "$A$1:$B$3")
        iter = exp.get_formula_tokens()
        self.assertEqual(len(iter), 1)
        tokens = [t for t in iter]
        self.assertEqual(str(tokens[0]), "$A$1:$B$3")

        sheet = doc.sheets[1]
        named_exps = sheet.get_named_expressions()
        self.assertEqual(named_exps.names(), {"MyRange",})
        self.assertEqual(len(named_exps), 1)

        named_exps_dict = {x[0]: x[1] for x in named_exps}
        exp = named_exps_dict["MyRange"]
        self.assertEqual(exp.formula, "$A$4:$B$5")
        iter = exp.get_formula_tokens()
        self.assertEqual(len(iter), 1)
        tokens = [t for t in iter]
        self.assertEqual(str(tokens[0]), "$A$4:$B$5")


if __name__ == '__main__':
    unittest.main()
