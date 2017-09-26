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

    def insert_cell(self, row, column, cell_type, cell_value, result):
        if row not in self.__rows:
            self.__rows[row] = collections.OrderedDict()

        row_data = self.__rows[row]

        if cell_type == "numeric":
            row_data[column] = float(cell_value)
        elif cell_type == "string":
            row_data[column] = self.__unescape_string_cell_value(cell_value)
        elif cell_type == "boolean":
            if cell_value == "true":
                row_data[column] = True
            elif cell_value == "false":
                row_data[column] = False
            else:
                raise RuntimeError("invalid boolean value: {}".format(cell_value))
        elif cell_type == "formula":
            row_data[column] = result
        else:
            raise RuntimeError("unhandled cell value type: {}".format(cell_type))

        # Update the data range.
        if row > self.__max_row:
            self.__max_row = row
        if column > self.__max_column:
            self.__max_column = column

    def __unescape_string_cell_value(self, v):
        if v[0] != '"' or v[-1] != '"':
            raise RuntimeError("string value is expected to be quoted.")

        v = v[1:-1]  # remove the outer quotes.

        buf = []
        escaped_char = False
        for c in v:
            if escaped_char:
                buf.append(c)
                escaped_char = False
                continue

            if c == '\\':
                escaped_char = True
                continue

            buf.append(c)

        return "".join(buf)


class ExpectedDocument(object):

    def __init__(self, filepath):
        self.sheets = []

        with open(filepath, "r") as f:
            for line in f.readlines():
                line = line.strip()
                self.__parse_line(line)

    def __parse_line(self, line):
        if not line:
            return

        vs = line.split(':')
        result = None
        if len(vs) == 3:
            pos, cell_type, cell_value = vs
        elif len(vs) == 4:
            # formula cell - cell_value contains formula expression.
            pos, cell_type, cell_value, result = vs
            result = float(result)  # string to float
        else:
            raise RuntimeError("line contains {} elements.".format(len(vs)))

        pos = Address(pos)

        if not self.sheets or self.sheets[-1].name != pos.sheet_name:
            self.sheets.append(ExpectedSheet(pos.sheet_name))

        self.sheets[-1].insert_cell(pos.row, pos.column, cell_type, cell_value, result)


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

    # Sometimes the actual document contains trailing empty sheets, which the
    # expected document does not store.
    self.assertTrue(len(expected.sheets))
    self.assertTrue(len(expected.sheets) <= len(doc.sheets))

    expected_sheets = {sh.name: sh for sh in expected.sheets}
    actual_sheets = {sh.name: sh for sh in doc.sheets}

    for sheet_name, actual_sheet in actual_sheets.items():
        if sheet_name in expected_sheets:
            expected_sheet = expected_sheets[sheet_name]
            self.assertEqual(expected_sheet.data_size, actual_sheet.data_size)
            for expected_row, actual_row in zip(expected_sheet.get_rows(), actual_sheet.get_rows()):
                self.assertEqual(expected_row, actual_row)
        else:
            # This sheet must be empty since it's not in the expected document.
            # Make sure it returns empty row set.
            rows = [row for row in actual_sheet.get_rows()]
            self.assertEqual(len(rows), 0)
