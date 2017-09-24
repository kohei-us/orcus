########################################################################
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
########################################################################

"""Collection of test cases shared between different file format types."""

import os
import os.path
import collections
import orcus


class Address(object):

    def __init__(self, pos_s):
        self.sheet_name, self.row, self.column = pos_s.split('/')
        self.row = int(self.row)
        self.column = int(self.column)

    def __repr__(self):
        return "(sheet={}; row={}, column={})".format(self.sheet_name, self.row, self.column)


class ExpectedSheet(object):

    def __init__(self, name):
        self.__name = name
        self.__rows = collections.OrderedDict()
        self.__max_column = 0
        self.__max_row = 0

    @property
    def name(self):
        return self.__name

    @property
    def data_size(self):
        return {"column": self.__max_column+1, "row": self.__max_row+1}

    def get_rows(self):
        rows = list()
        for i in range(self.__max_row+1):
            row = [None for _ in range(self.__max_column+1)]
            if i in self.__rows:
                for col_pos, cell in self.__rows[i].items():
                    row[col_pos] = cell
            rows.append(tuple(row))
        return tuple(rows)

    def insert_cell(self, row, column, cell_type, cell_value):
        if row not in self.__rows:
            self.__rows[row] = collections.OrderedDict()

        row_data = self.__rows[row]

        if cell_type == "numeric":
            row_data[column] = float(cell_value)
        elif cell_type == "string":
            # string value is quoted.
            if cell_value[0] != '"' or cell_value[-1] != '"':
                raise RuntimeError("string value is expected to be quoted.")
            row_data[column] = cell_value[1:-1]
        else:
            raise RuntimeError("unhandled cell value type: {}".format(cell_type))

        # Update the data range.
        if row > self.__max_row:
            self.__max_row = row
        if column > self.__max_column:
            self.__max_column = column


class ExpectedDocument(object):

    def __init__(self, filepath):
        self.sheets = []

        with open(filepath, "r") as f:
            for line in f.readlines():
                line = line.strip()
                self.__parse_line(line)

    def __parse_line(self, line):
        pos, cell_type, cell_value = line.split(':')
        pos = Address(pos)

        if not self.sheets or self.sheets[-1].name != pos.sheet_name:
            self.sheets.append(ExpectedSheet(pos.sheet_name))

        self.sheets[-1].insert_cell(pos.row, pos.column, cell_type, cell_value)


def run_test_dir(self, test_dir, mod_loader):
    """Run test case for loading a file into a document.

    :param test_dir: test directory that contains an input file (whose base
       name is 'input') and a content check file (check.txt).
    :param mod_loader: module object that contains function called 'read'.
    """

    print("test directory: {}".format(test_dir))
    expected = ExpectedDocument(os.path.join(test_dir, "check.txt"))

    # Find the input file to load.
    input_file = None
    for file_name in os.listdir(test_dir):
        name, ext = os.path.splitext(file_name)
        if name == "input":
            input_file = os.path.join(test_dir, file_name)
            break

    print("input file: {}".format(input_file))
    self.assertIsNot(input_file, None)

    with open(input_file, "rb") as f:
        doc = mod_loader.read(f)

    self.assertIsInstance(doc, orcus.Document)
    self.assertEqual(len(expected.sheets), len(doc.sheets))

    for expected_sheet, actual_sheet in zip(expected.sheets, doc.sheets):
        self.assertEqual(expected_sheet.name, actual_sheet.name)
        self.assertEqual(expected_sheet.data_size, actual_sheet.data_size)

        for expected_row, actual_row in zip(expected_sheet.get_rows(), actual_sheet.get_rows()):
            self.assertEqual(expected_row, actual_row)

