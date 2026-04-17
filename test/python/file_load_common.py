########################################################################
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
########################################################################

"""Collection of test cases shared between different file format types."""

import collections
import math
from pathlib import Path
import orcus


class Address(object):

    def __init__(self, pos_s):
        self.sheet_name, self.row, self.column = pos_s.split('/')
        self.row = int(self.row)
        self.column = int(self.column)

    def __repr__(self):
        return f"(sheet={self.sheet_name}; row={self.row}, column={self.column})"


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
            row = [(orcus.CellType.EMPTY, None) for _ in range(self.__max_column+1)]
            if i in self.__rows:
                for col_pos, cell in self.__rows[i].items():
                    row[col_pos] = cell
            rows.append(tuple(row))
        return tuple(rows)

    def insert_cell(self, row, column, cell_type, cell_value, result, result_places):
        if row not in self.__rows:
            self.__rows[row] = collections.OrderedDict()

        row_data = self.__rows[row]

        if cell_type == "numeric":
            row_data[column] = (orcus.CellType.NUMERIC, float(cell_value), result_places)
        elif cell_type == "string":
            row_data[column] = (orcus.CellType.STRING, self.__unescape_string_cell_value(cell_value))
        elif cell_type == "boolean":
            if cell_value == "true":
                row_data[column] = (orcus.CellType.BOOLEAN, True)
            elif cell_value == "false":
                row_data[column] = (orcus.CellType.BOOLEAN, False)
            else:
                raise RuntimeError(f"invalid boolean value: {cell_value}")
        elif cell_type == "formula":
            row_data[column] = (orcus.CellType.FORMULA, result, cell_value, result_places)
        else:
            raise RuntimeError(f"unhandled cell value type: {cell_type}")

        # Update the data range.
        if row > self.__max_row:
            self.__max_row = row
        if column > self.__max_column:
            self.__max_column = column

    def __unescape_string_cell_value(self, v):
        if v[0] != '"' or v[-1] != '"':
            raise RuntimeError("string value is expected to be quoted.")

        v = v[1:-1]  # remove the outer quotes.

        escape_map = {"n": "\n", "t": "\t", "\\": "\\"}

        buf = []
        escaped_char = False
        for c in v:
            if escaped_char:
                buf.append(escape_map.get(c, c))
                escaped_char = False
                continue

            if c == '\\':
                escaped_char = True
                continue

            buf.append(c)

        return "".join(buf)


class ExpectedDocument(object):

    @staticmethod
    def _decimal_places(s):
        idx = s.find('.')
        return len(s) - idx - 1 if idx >= 0 else 0

    def __init__(self, filepath):
        self.sheets = []

        with filepath.open("r") as f:
            for line in f.readlines():
                line = line.strip()
                self.__parse_line(line)

    def __parse_line(self, line):
        if not line:
            return

        # Split the line into 3 parts - position, cell type and the value.
        # Note that a valid formula expression may contain ':', so we cannot
        # simply split the line by ':'.

        parts = list()
        idx = line.find(':')
        while idx >= 0:
            parts.append(line[:idx])
            line = line[idx+1:]
            if len(parts) == 2:
                # Append the rest.
                parts.append(line)
                break

            idx = line.find(':')

        if len(parts) != 3:
            raise RuntimeError(
                "line is expected to contain 3 parts, but not all parts are identified.")

        if parts[1] in ("merge-width", "merge-height"):
            return

        pos, cell_type, cell_value = parts[0], parts[1], parts[2]
        result = None
        result_places = 0
        if cell_type == "numeric":
            result_places = self._decimal_places(cell_value)
        elif cell_type == "formula":
            # Split the cell value into formula expression and result.
            idx = cell_value.rfind(':')
            if idx < 0:
                raise RuntimeError("formula line is expected to contain a result value.")
            result_str = cell_value[idx+1:]
            cell_value = cell_value[:idx]
            result_places = 0
            try:
                result = float(result_str)
                result_places = self._decimal_places(result_str)
            except ValueError:
                result = result_str

        pos = Address(pos)

        if not self.sheets or self.sheets[-1].name != pos.sheet_name:
            self.sheets.append(ExpectedSheet(pos.sheet_name))

        self.sheets[-1].insert_cell(pos.row, pos.column, cell_type, cell_value, result, result_places)


def compare_cells(expected, actual):
    type = expected[0]

    if type != actual.type:
        return False, f"{type} is expected, but {actual.type} is found"

    if type == orcus.CellType.EMPTY:
        return True, None

    if type == orcus.CellType.NUMERIC:
        tol = 0.5 * 10 ** -expected[2]
        if not math.isclose(expected[1], actual.value, abs_tol=tol, rel_tol=0):
            return False, f"expected value is {expected[1]} but {actual.value} is found"
        return True, None

    if type in (orcus.CellType.BOOLEAN, orcus.CellType.STRING):
        if expected[1] == actual.value:
            return True, None
        else:
            return False, f"expected value is {expected[1]} but {actual.value} is found"

    if type == orcus.CellType.FORMULA:
        if isinstance(expected[1], float):
            tol = 0.5 * 10 ** -expected[3]
            if not math.isclose(expected[1], actual.value, abs_tol=tol, rel_tol=0):
                return False, f"expected value is {expected[1]} but {actual.value} is found"
        elif expected[1] != actual.value:
            return False, f"expected value is {expected[1]} but {actual.value} is found"
        if expected[2] != actual.formula:
            return False, f"expected formula is {expected[2]} but {actual.formula} is found"
        return True, None

    return False, "cell comparison result yielded false for unknown reason"


class DocLoader:

    def __init__(self, mod_loader):
        self._mod_loader = mod_loader

    def load(self, filepath, recalc):
        return self._mod_loader.read(filepath.open("rb"), recalc=recalc)

    def load_from_value(self, filepath):
        return self._mod_loader.read(filepath.read_bytes(), recalc=False)


def run_test_dir(test_dir, doc_loader):
    """Run test case for loading a file into a document.

    :param test_dir: test directory that contains an input file (whose base
       name is 'input') and a content check file (check.txt).
    :param doc_loader: DocLoader instance that loads the input file.
    """

    test_dir = Path(test_dir)
    print(f"test directory: {test_dir}")
    expected = ExpectedDocument(test_dir / "check.txt")

    # Find the input file to load.
    input_file = None
    for p in test_dir.iterdir():
        if p.stem == "input":
            input_file = p
            break

    print(f"input file: {input_file}")
    assert input_file is not None

    doc = doc_loader.load(input_file, True)
    assert isinstance(doc, orcus.Document)

    # Sometimes the actual document contains trailing empty sheets, which the
    # expected document does not store.
    assert len(expected.sheets) > 0
    assert len(expected.sheets) <= len(doc.sheets)

    expected_sheets = {sh.name: sh for sh in expected.sheets}
    actual_sheets = {sh.name: sh for sh in doc.sheets}

    for sheet_name, actual_sheet in actual_sheets.items():
        if sheet_name in expected_sheets:
            expected_sheet = expected_sheets[sheet_name]
            assert expected_sheet.data_size == actual_sheet.data_size
            for expected_row, actual_row in zip(expected_sheet.get_rows(), actual_sheet.get_rows()):
                for expected, actual in zip(expected_row, actual_row):
                    result, err = compare_cells(expected, actual)
                    assert result, err
        else:
            # This sheet must be empty since it's not in the expected document.
            # Make sure it returns empty row set.
            rows = [row for row in actual_sheet.get_rows()]
            assert len(rows) == 0

    # Also make sure the document loads fine without recalc.
    doc = doc_loader.load(input_file, False)
    assert isinstance(doc, orcus.Document)

    # Make sure the document loads from in-memory value.
    doc = doc_loader.load_from_value(input_file)
    assert isinstance(doc, orcus.Document)
