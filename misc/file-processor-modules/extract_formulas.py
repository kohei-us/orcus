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

from common import config


FORMULAS_FILENAME_XML = f"{config.prefix_skip}formulas.xml"


def to_string(obj):
    if isinstance(obj, orcus.FormulaTokenType):
        n = len("FormulaTokenType.")
        s = str(obj)
        return s[n:].lower()

    return str(obj)


def escape_str(s):
    buf = []
    for c in s:
        if c == '"':
            buf.append("&quot;")
        elif c == '&':
            buf.append("&amp;")
        elif c == '<':
            buf.append("&lt;")
        elif c == '>':
            buf.append("&gt;")
        elif c == "'":
            buf.append("&apos;")
        else:
            buf.append(c)
    return "".join(buf)


def process_document(filepath, doc):
    return process_document_direct_xml(filepath, doc)


def process_document_direct_xml(filepath, doc):

    def write_tokens(iter, f):
        for token in iter:
            f.write(f'<token s="{escape_str(str(token))}" type="{to_string(token.type)}"/>')

    def write_named_exps(iter, f, scope):
        for name, exp in iter:
            f.write(f'<named-expression name="{escape_str(name)}" origin="{escape_str(exp.origin)}" formula="{escape_str(exp.formula)}" scope="{scope}">')
            write_tokens(exp.get_formula_tokens(), f)
            f.write("</named-expression>")

    outpath = f"{filepath}{FORMULAS_FILENAME_XML}"
    with open(outpath, "w") as f:
        output_buffer = list()
        f.write(f'<doc filepath="{escape_str(filepath)}"><sheets count="{len(doc.sheets)}">')
        for sheet in doc.sheets:
            f.write(f'<sheet name="{escape_str(sheet.name)}"/>')
        f.write("</sheets>")
        f.write("<named-expressions>")

        write_named_exps(doc.get_named_expressions(), f, "global")
        for sheet in doc.sheets:
            write_named_exps(sheet.get_named_expressions(), f, "sheet")

        f.write("</named-expressions>")

        f.write("<formulas>")

        for sheet in doc.sheets:
            output_buffer.append(f"* sheet: {sheet.name}")
            for row_pos, row in enumerate(sheet.get_rows()):
                for col_pos, cell in enumerate(row):
                    if cell.type == orcus.CellType.FORMULA:
                        f.write(f'<formula sheet="{escape_str(sheet.name)}" row="{row_pos}" column="{col_pos}" formula="{escape_str(cell.formula)}" valid="true">')
                        write_tokens(cell.get_formula_tokens(), f)
                        f.write("</formula>")
                    elif cell.type == orcus.CellType.FORMULA_WITH_ERROR:
                        tokens = [str(t) for t in cell.get_formula_tokens()]
                        f.write(f'<formula sheet="{escape_str(sheet.name)}" row="{row_pos}" column="{col_pos}" formula="{escape_str(tokens[1])}" error="{escape_str(tokens[2])}" valid="false"/>')
                    else:
                        continue

        f.write("</formulas>")
        f.write("</doc>")

        return output_buffer


def process_document_xml_etree(filepath, doc):
    import xml.etree.ElementTree as ET

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

    add_tokens = False
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
                    if "formula_tokens" in data:
                        elem.attrib["token-count"] = str(len(data["formula_tokens"]))
                        for token in data["formula_tokens"]:
                            elem_token = ET.SubElement(elem, "token")
                            elem_token.attrib["s"] = token[0]
                            elem_token.attrib["type"] = token[1]
                else:
                    # invalid formula
                    elem.attrib["error"] = data["error"]

    outpath = f"{filepath}{FORMULAS_FILENAME_XML}"
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
            if not filename.endswith(FORMULAS_FILENAME_XML):
                continue

            inpath = os.path.join(root, filename)
            outpath = os.path.join(args.output, f"{i+1:04}.xml")
            shutil.copyfile(inpath, outpath)
            i += 1


if __name__ == "__main__":
    main()
