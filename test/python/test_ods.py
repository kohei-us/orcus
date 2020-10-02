#!/usr/bin/env python3
########################################################################
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
########################################################################

import unittest
from pathlib import Path

from orcus import ods, FormulaTokenType, FormulaTokenOp

import file_load_common as common


class TestCase(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        # base directory for ods test files.
        basedir = Path(__file__).parent / ".." / "ods"
        cls.basedir = basedir.resolve()

    def test_import(self):
        test_dirs = ("raw-values-1", "formula-1", "formula-2")
        for test_dir in test_dirs:
            test_dir = self.basedir / test_dir
            common.run_test_dir(self, test_dir, common.DocLoader(ods))

    def test_formula_tokens_1(self):
        filepath = self.basedir / "formula-1" / "input.ods"
        with open(filepath, "rb") as f:
            doc = ods.read(f, recalc=False)

        self.assertEqual(len(doc.sheets), 1)

        # The 'Formula' sheet contains 4 formula cells in A1:A4.
        sheet = doc.sheets[0]
        self.assertEqual(sheet.name, "Formula")
        rows = [row for row in sheet.get_rows()]
        self.assertEqual(len(rows), 4)

        expected = ("1*2", "12/3", "AVERAGE($A1:A$2)", "SUM($A$1:$A$3)")
        for row, expected_formula in zip(sheet.get_rows(), expected):
            c = row[0]
            self.assertEqual(c.formula, expected_formula)

        expected = (
            (
                ("1", FormulaTokenType.VALUE, FormulaTokenOp.VALUE),
                ("*", FormulaTokenType.OPERATOR, FormulaTokenOp.MULTIPLY),
                ("2", FormulaTokenType.VALUE, FormulaTokenOp.VALUE)
            ),
            (
                ("12", FormulaTokenType.VALUE, FormulaTokenOp.VALUE),
                ("/", FormulaTokenType.OPERATOR, FormulaTokenOp.DIVIDE),
                ("3", FormulaTokenType.VALUE, FormulaTokenOp.VALUE)
            ),
            (
                ("AVERAGE", FormulaTokenType.FUNCTION, FormulaTokenOp.FUNCTION),
                ("(", FormulaTokenType.OPERATOR, FormulaTokenOp.OPEN),
                ("$A1:A$2", FormulaTokenType.REFERENCE, FormulaTokenOp.RANGE_REF),
                (")", FormulaTokenType.OPERATOR, FormulaTokenOp.CLOSE)
            ),
            (
                ("SUM", FormulaTokenType.FUNCTION, FormulaTokenOp.FUNCTION),
                ("(", FormulaTokenType.OPERATOR, FormulaTokenOp.OPEN),
                ("$A$1:$A$3", FormulaTokenType.REFERENCE, FormulaTokenOp.RANGE_REF),
                (")", FormulaTokenType.OPERATOR, FormulaTokenOp.CLOSE)
            ),
        )

        for row, expected_formula_tokens in zip(sheet.get_rows(), expected):
            c = row[0]
            iter = c.get_formula_tokens()
            for token, expected_token in zip(iter, expected_formula_tokens):
                self.assertEqual(str(token), expected_token[0])
                self.assertEqual(token.type, expected_token[1])
                self.assertEqual(token.op, expected_token[2])

    def test_formula_tokens_2(self):
        filepath = self.basedir / "formula-2" / "input.ods"
        with open(filepath, "rb") as f:
            doc = ods.read(f, recalc=False)

        self.assertEqual(len(doc.sheets), 1)

        expected = (
            (
                ("CONCATENATE", FormulaTokenType.FUNCTION, FormulaTokenOp.FUNCTION),
                ("(", FormulaTokenType.OPERATOR, FormulaTokenOp.OPEN),
                ("A2", FormulaTokenType.REFERENCE, FormulaTokenOp.SINGLE_REF),
                (",", FormulaTokenType.OPERATOR, FormulaTokenOp.SEP),
                ('" "', FormulaTokenType.VALUE, FormulaTokenOp.STRING),
                (",", FormulaTokenType.OPERATOR, FormulaTokenOp.SEP),
                ("B2", FormulaTokenType.REFERENCE, FormulaTokenOp.SINGLE_REF),
                (")", FormulaTokenType.OPERATOR, FormulaTokenOp.CLOSE),
            ),
            (
                ("CONCATENATE", FormulaTokenType.FUNCTION, FormulaTokenOp.FUNCTION),
                ("(", FormulaTokenType.OPERATOR, FormulaTokenOp.OPEN),
                ("A3", FormulaTokenType.REFERENCE, FormulaTokenOp.SINGLE_REF),
                (",", FormulaTokenType.OPERATOR, FormulaTokenOp.SEP),
                ('" "', FormulaTokenType.VALUE, FormulaTokenOp.STRING),
                (",", FormulaTokenType.OPERATOR, FormulaTokenOp.SEP),
                ("B3", FormulaTokenType.REFERENCE, FormulaTokenOp.SINGLE_REF),
                (")", FormulaTokenType.OPERATOR, FormulaTokenOp.CLOSE),
            ),
            (
                ("CONCATENATE", FormulaTokenType.FUNCTION, FormulaTokenOp.FUNCTION),
                ("(", FormulaTokenType.OPERATOR, FormulaTokenOp.OPEN),
                ("A4", FormulaTokenType.REFERENCE, FormulaTokenOp.SINGLE_REF),
                (",", FormulaTokenType.OPERATOR, FormulaTokenOp.SEP),
                ('" "', FormulaTokenType.VALUE, FormulaTokenOp.STRING),
                (",", FormulaTokenType.OPERATOR, FormulaTokenOp.SEP),
                ("B4", FormulaTokenType.REFERENCE, FormulaTokenOp.SINGLE_REF),
                (")", FormulaTokenType.OPERATOR, FormulaTokenOp.CLOSE),
            ),
            (
                ("CONCATENATE", FormulaTokenType.FUNCTION, FormulaTokenOp.FUNCTION),
                ("(", FormulaTokenType.OPERATOR, FormulaTokenOp.OPEN),
                ("A5", FormulaTokenType.REFERENCE, FormulaTokenOp.SINGLE_REF),
                (",", FormulaTokenType.OPERATOR, FormulaTokenOp.SEP),
                ('" "', FormulaTokenType.VALUE, FormulaTokenOp.STRING),
                (",", FormulaTokenType.OPERATOR, FormulaTokenOp.SEP),
                ("B5", FormulaTokenType.REFERENCE, FormulaTokenOp.SINGLE_REF),
                (")", FormulaTokenType.OPERATOR, FormulaTokenOp.CLOSE),
            ),
            (
                ("CONCATENATE", FormulaTokenType.FUNCTION, FormulaTokenOp.FUNCTION),
                ("(", FormulaTokenType.OPERATOR, FormulaTokenOp.OPEN),
                ("A6", FormulaTokenType.REFERENCE, FormulaTokenOp.SINGLE_REF),
                (",", FormulaTokenType.OPERATOR, FormulaTokenOp.SEP),
                ('" "', FormulaTokenType.VALUE, FormulaTokenOp.STRING),
                (",", FormulaTokenType.OPERATOR, FormulaTokenOp.SEP),
                ("B6", FormulaTokenType.REFERENCE, FormulaTokenOp.SINGLE_REF),
                (")", FormulaTokenType.OPERATOR, FormulaTokenOp.CLOSE),
            ),
            (
                ("CONCATENATE", FormulaTokenType.FUNCTION, FormulaTokenOp.FUNCTION),
                ("(", FormulaTokenType.OPERATOR, FormulaTokenOp.OPEN),
                ("A7", FormulaTokenType.REFERENCE, FormulaTokenOp.SINGLE_REF),
                (",", FormulaTokenType.OPERATOR, FormulaTokenOp.SEP),
                ('" "', FormulaTokenType.VALUE, FormulaTokenOp.STRING),
                (",", FormulaTokenType.OPERATOR, FormulaTokenOp.SEP),
                ("B7", FormulaTokenType.REFERENCE, FormulaTokenOp.SINGLE_REF),
                (")", FormulaTokenType.OPERATOR, FormulaTokenOp.CLOSE),
            ),
        )

        # Check cells in column C.
        rows = [row for row in doc.sheets[0].get_rows()]
        for row, expected_tokens in zip(rows[1:], expected):  # skip the header row
            tokens = row[2].get_formula_tokens()
            for token, expected_token in zip(tokens, expected_tokens):
                self.assertEqual(str(token), expected_token[0])
                self.assertEqual(token.type, expected_token[1])
                self.assertEqual(token.op, expected_token[2])


if __name__ == '__main__':
    unittest.main()
