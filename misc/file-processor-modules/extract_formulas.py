#!/usr/bin/env python3
########################################################################
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
########################################################################

import orcus
import json
import os
import os.path
import sys

from common import config


FORMULAS_JSON_FILENAME = f"{config.prefix_skip}formulas.json"


def process_document(filepath, doc):
    """File processor callback function."""

    sheet_names = list()
    formula_cells =  list()

    for sh in doc.sheets:
        sheet_names.append(sh.name)

        for row_pos, row in enumerate(sh.get_rows()):
            for col_pos, cell in enumerate(row):
                if cell.type != orcus.CellType.FORMULA or not cell.formula:
                    # Skip this cell.
                    continue

                formula_cells.append(
                    dict(sheet=sh.name, row=row_pos, column=col_pos, formula=cell.formula, formula_tokens=cell.formula_tokens))

    data = dict()
    data["filepath"] = filepath
    data["sheets"] = sheet_names
    data["formulas"] = formula_cells

    output_buffer = list()
    for sn in data["sheets"]:
        output_buffer.append(f"* sheet: {sn}")

    n = len(data["formulas"])
    output_buffer.append(f"* formula cell count: {n}")

    dirpath = os.path.dirname(filepath)
    outpath = os.path.join(dirpath, FORMULAS_JSON_FILENAME)
    with open(outpath, "w") as f:
        s = json.dumps(data)
        f.write(s)

    return output_buffer


def main():
    import argparse
    parser = argparse.ArgumentParser()
    parser.add_argument("-o", "--output", type=argparse.FileType("w"), help="Output file to write all the formula data to.")
    parser.add_argument("rootdir", help="Root directory from which to traverse for the formula files.")
    args = parser.parse_args()

    data = list()
    for root, dir, files in os.walk(args.rootdir):
        for filename in files:
            if filename != FORMULAS_JSON_FILENAME:
                continue

            filepath = os.path.join(root, filename)
            with open(filepath, "r") as f:
                data.append(json.loads(f.read()))

    output = args.output if args.output else sys.stdout
    import pprint
    pprint.pprint(data, stream=output, width=256)


if __name__ == "__main__":
    main()
