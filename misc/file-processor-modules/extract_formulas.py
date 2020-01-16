########################################################################
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
########################################################################

import orcus
import json
import os.path


def process_document(config, filepath, doc):
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
    outpath = os.path.join(dirpath, f"{config.prefix_skip}formulas.json")
    with open(outpath, "w") as f:
        s = json.dumps(data)
        f.write(s)

    return output_buffer
