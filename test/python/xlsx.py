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

    def test_raw_values_1(self):
        filepath = os.path.join(self.basedir, "raw-values-1", "input.xlsx")
        with open(filepath, "rb") as f:
            doc = xlsx.read(f)

        common.test_raw_values_1(self, doc)


if __name__ == '__main__':
    unittest.main()
