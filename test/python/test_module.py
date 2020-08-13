#!/usr/bin/env python3
########################################################################
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
########################################################################

import unittest
import orcus
import os.path
import json
from pathlib import Path


class ModuleTest(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        with open(Path(__file__).parent / "env.json", "r") as f:
            cls.env = json.load(f)

    def test_version(self):
        s = orcus.__version__
        expected = f"{self.env['version-major']}.{self.env['version-minor']}.{self.env['version-micro']}"
        self.assertEqual(expected, s)

    def test_detect_format(self):
        test_root_dir = os.path.join(os.path.dirname(__file__), "..")

        checks = (
            (("ods", "raw-values-1", "input.ods"), orcus.FormatType.ODS),
            (("xlsx", "raw-values-1", "input.xlsx"), orcus.FormatType.XLSX),
            (("xls-xml", "raw-values-1", "input.xml"), orcus.FormatType.XLS_XML),
            (("gnumeric", "raw-values-1", "input.gnumeric"), orcus.FormatType.GNUMERIC),
        )

        for check in checks:
            filepath = os.path.join(test_root_dir, *check[0])
            with open(filepath, "rb") as f:
                # Pass the file object directly.
                fmt = orcus.detect_format(f)
                self.assertEqual(check[1], fmt)

                # Pass the bytes.
                f.seek(0)
                bytes = f.read()
                fmt = orcus.detect_format(bytes)
                self.assertEqual(check[1], fmt)


if __name__ == '__main__':
    unittest.main()
