#!/usr/bin/env python3
########################################################################
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
########################################################################

import orcus
import os
import os.path
import sys
import enum
import shutil
import xml.etree.ElementTree as ET

from common import config


FORMULAS_FILENAME_XML = f"{config.prefix_skip}formulas.xml"


def to_string(obj):
    if isinstance(obj, orcus.FormulaTokenType):
        n = len("FormulaTokenType.")
        s = str(obj)
        return s[n:].lower()

    return str(obj)


def process_formula_tokens(obj):
    formula_tokens = list()
    for token in obj.get_formula_tokens():
        formula_tokens.append([str(token), to_string(token.type)])
    return formula_tokens


def add_named_expression(parent, name, exp, add_tokens, scope):
    elem = ET.SubElement(parent, "named-expression")
    elem.attrib["name"] = name
    elem.attrib["formula"] = exp.formula
    elem.attrib["scope"] = scope
    elem.attrib["origin"] = exp.origin

    if add_tokens:
        tokens = process_formula_tokens(exp)
        for token in tokens:
            elem_token = ET.SubElement(elem, "token")
            elem_token.attrib["s"] = token[0]
            elem_token.attrib["type"] = token[1]


def process_document(filepath, doc):

    add_tokens = True
    output_buffer = list()

    elem_doc = ET.Element("doc")
    elem_doc.attrib["filepath"] = filepath
    elem_sheets = ET.SubElement(elem_doc, "sheets")
    elem_sheets.attrib["count"] = str(len(doc.sheets))

    elem_named_exps = ET.SubElement(elem_doc, "named-expressions")

    for name, exp in doc.get_named_expressions():
        add_named_expression(elem_named_exps, name, exp, add_tokens, "global")

    elem_formulas = ET.SubElement(elem_doc, "formulas")

    for sheet in doc.sheets:
        for name, exp in sheet.get_named_expressions():
            add_named_expression(elem_named_exps, name, exp, add_tokens, "sheet")

        elem = ET.SubElement(elem_sheets, "sheet")
        elem.attrib["name"] = sheet.name
        output_buffer.append(f"* sheet: {sheet.name}")

        for row_pos, row in enumerate(sheet.get_rows()):
            for col_pos, cell in enumerate(row):
                data = dict(sheet=sheet.name, row=row_pos, column=col_pos)
                if cell.type == orcus.CellType.FORMULA:
                    data.update({
                        "valid": True,
                        "formula": cell.formula,
                    })

                    if add_tokens:
                        data["formula_tokens"] = process_formula_tokens(cell)

                elif cell.type == orcus.CellType.FORMULA_WITH_ERROR:
                    tokens = [str(x) for x in cell.get_formula_tokens()]
                    data.update({
                        "valid": False,
                        "formula": tokens[1],  # original formula string
                        "error": tokens[2],
                    })
                else:
                    continue

                elem = ET.SubElement(elem_formulas, "formula")
                elem.attrib["sheet"] = data["sheet"]
                elem.attrib["row"] = str(data["row"])
                elem.attrib["column"] = str(data["column"])
                elem.attrib["s"] = data["formula"]
                elem.attrib["valid"] = "true" if data["valid"] else "false"
                if data["valid"]:
                    elem.attrib["token-count"] = str(len(data["formula_tokens"]))
                    if "formula_tokens" in data:
                        for token in data["formula_tokens"]:
                            elem_token = ET.SubElement(elem, "token")
                            elem_token.attrib["s"] = token[0]
                            elem_token.attrib["type"] = token[1]
                else:
                    # invalid formula
                    elem.attrib["error"] = data["error"]

    dirpath = os.path.dirname(filepath)
    outpath = os.path.join(dirpath, FORMULAS_FILENAME_XML)
    with open(outpath, "w") as f:
        s = ET.tostring(elem_doc, "utf-8").decode("utf-8")
        f.write(s)

    return output_buffer


def main():
    import argparse
    parser = argparse.ArgumentParser()
    parser.add_argument("-o", "--output", type=str, required=True, help="Output directory to write all the formula data to.")
    parser.add_argument("rootdir", help="Root directory from which to traverse for the formula files.")
    args = parser.parse_args()

    os.makedirs(args.output, exist_ok=True)

    i = 0
    for root, dir, files in os.walk(args.rootdir):
        for filename in files:
            if filename != FORMULAS_FILENAME_XML:
                continue

            inpath = os.path.join(root, filename)
            outpath = os.path.join(args.output, f"{i+1:04}.xml")
            shutil.copyfile(inpath, outpath)
            i += 1


if __name__ == "__main__":
    main()
