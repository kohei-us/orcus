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
import enum

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


class OutputFormat(enum.Enum):
    JSON = "json"
    XML = "xml"


def dump_json_as_xml(data, stream):
    import xml.etree.ElementTree as ET
    root = ET.Element("docs")

    for doc in data:
        elem_doc = ET.SubElement(root, "doc")
        elem_doc.attrib["filepath"] = doc["filepath"]
        elem_sheets = ET.SubElement(elem_doc, "sheets")
        for sheet in doc["sheets"]:
            elem = ET.SubElement(elem_sheets, "sheet")
            elem.attrib["name"] = sheet

        elem_formulas = ET.SubElement(elem_doc, "formulas")
        for formula_cell in doc["formulas"]:
            elem = ET.SubElement(elem_formulas, "formula")
            elem.attrib["sheet"] = formula_cell["sheet"]
            elem.attrib["row"] = str(formula_cell["row"])
            elem.attrib["column"] = str(formula_cell["column"])
            elem.attrib["s"] = formula_cell["formula"]
            for token in formula_cell["formula_tokens"]:
                elem_token = ET.SubElement(elem, "token")
                elem_token.attrib["s"] = token

    s = ET.tostring(root, "utf-8").decode("utf-8")
    from xml.dom import minidom
    s = minidom.parseString(s).toprettyxml(indent="    ")
    stream.write(s)


def main():
    import argparse
    parser = argparse.ArgumentParser()
    parser.add_argument("-o", "--output", type=argparse.FileType("w"), help="Output file to write all the formula data to.")
    parser.add_argument("-f", "--format", type=OutputFormat, default=OutputFormat.JSON, help="Output file format.")
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
    if args.format == OutputFormat.JSON:
        import pprint
        pprint.pprint(data, stream=output, width=256)
    elif args.format == OutputFormat.XML:
        dump_json_as_xml(data, output)


if __name__ == "__main__":
    main()
