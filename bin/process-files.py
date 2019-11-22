#!/usr/bin/env python3

import argparse
import os
import os.path
import sys
import string
import pathlib
import enum

import orcus


def sanitize_string(s):
    """Replace non-printable characters with \\x[value]."""

    buf = list()
    for c in s:
        if c in string.printable:
            buf.append(c)
        else:
            buf.append(f"\\x{ord(c):02X}")

    return "".join(buf)


class LoadStatus(enum.Enum):
    SUCCESS = 0
    FAILURE = 1
    SKIPPED = 2


def load_doc(bytes):

    buf = list()

    try:
        format_type = orcus.detect_format(bytes)
    except Exception as e:
        buf.append(str(e))
        status = LoadStatus.SKIPPED
        return None, status, buf

    buf.append(f"* format type: {format_type}")
    buf.append(f"* size: {len(bytes)} bytes")

    doc = None

    try:
        status = LoadStatus.SUCCESS
        if format_type == "ods":
            from orcus import ods
            doc = ods.read(bytes)
        elif format_type == "xlsx":
            from orcus import xlsx
            doc = xlsx.read(bytes)
        else:
            buf.append(f"unhandled format type: {format_type}")
            status = LoadStatus.SKIPPED

        return doc, status, buf

    except Exception as e:
        buf.append(f"{e.__class__.__name__}: {e}")
        status = LoadStatus.FAILURE
        return None, status, buf


def print_doc_preview(doc):
    buf = list()
    for sh in doc.sheets:
        buf.append(f"sheet: {sh.name}")
        for i, row in enumerate(sh.get_rows()):
            if i > 10:
                buf.append("...")
                break
            buf.append(f"row {i}: {row}")
    return buf


def main():
    parser = argparse.ArgumentParser(
        description="""This script allows you to load a collection of input files
        for testing purpuses.""")
    parser.add_argument(
        "rootdir", metavar="ROOT-DIR",
        help="Root directory below which to recursively find and process test files.")
    args = parser.parse_args()

    file_count = 0
    good_fileext = "orcus-pf.good"
    bad_fileext = "orcus-pf.bad"
    out_fileext = "orcus-pf.out"

    for root, dir, files in os.walk(args.rootdir):
        for filename in files:
            if filename.endswith(out_fileext) or filename.endswith(good_fileext) or filename.endswith(bad_fileext):
                continue

            inpath = os.path.join(root, filename)
            outpath = f"{inpath}.{out_fileext}"
            print(file_count, sanitize_string(inpath), flush=True)
            file_count += 1
            buf = list()

            good_filepath = f"{inpath}.{good_fileext}"
            bad_filepath = f"{inpath}.{bad_fileext}"

            if os.path.isfile(good_filepath) or os.path.isfile(bad_filepath):
                print("already processed. skipping...")
                continue

            success = False
            with open(inpath, 'rb') as f:
                bytes = f.read()

            doc, status, output = load_doc(bytes)
            buf.extend(output)
            if doc:
                buf.extend(print_doc_preview(doc))

            with open(outpath, "w") as f:
                f.write("\n".join(buf))
            print("\n".join(buf))

            if status == LoadStatus.SUCCESS:
                pathlib.Path(good_filepath).touch()
            elif status == LoadStatus.FAILURE:
                pathlib.Path(bad_filepath).touch()


if __name__ == "__main__":
    main()
