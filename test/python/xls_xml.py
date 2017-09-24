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

from orcus import xls_xml

import file_load_common as common


class TestCase(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        # base directory for ods test files.
        basedir = os.path.join(os.path.dirname(__file__), "..", "xls-xml")
        cls.basedir = os.path.normpath(basedir)

    def test_raw_values_1(self):
        filepath = os.path.join(self.basedir, "raw-values-1", "input.xml")
        with open(filepath, "rb") as f:
            doc = xls_xml.read(f)

        common.test_raw_values_1(self, doc)


if __name__ == '__main__':
    unittest.main()
