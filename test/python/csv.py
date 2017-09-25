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

from orcus import csv

import file_load_common as common


class TestCase(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        # base directory for ods test files.
        basedir = os.path.join(os.path.dirname(__file__), "..", "csv")
        cls.basedir = os.path.normpath(basedir)

    def test_import(self):
        test_dirs = ("simple-numbers", "normal-quotes", "double-quotes", "quoted-with-delim")
        for test_dir in test_dirs:
            test_dir = os.path.join(self.basedir, test_dir)
            common.run_test_dir(self, test_dir, csv)


if __name__ == '__main__':
    unittest.main()
