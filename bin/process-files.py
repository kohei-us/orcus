#!/usr/bin/env python3

import argparse
import os
import os.path
import sys
import string

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


def main():
    parser = argparse.ArgumentParser(
        description="""This script allows you to load a collection of input files
        for testing purpuses.""")
    parser.add_argument(
        "rootdir", metavar="ROOT-DIR",
        help="Root directory below which to recursively find and process test files.")
    args = parser.parse_args()

    file_count = 0
    for root, dir, files in os.walk(args.rootdir):
        for filename in files:
            filepath = os.path.join(root, filename)
            print(file_count, sanitize_string(filepath), flush=True)
            file_count += 1

            with open(filepath, 'rb') as f:
                bytes = f.read()
                try:
                    format_type = orcus.detect_format(bytes)
                except:
                    continue
                print(f"* format type: {format_type}")
                print(f"* size: {len(bytes)} bytes")

                doc = None

                try:
                    if format_type == "ods":
                        from orcus import ods
                        doc = ods.read(bytes)
                    elif format_type == "xlsx":
                        from orcus import xlsx
                        doc = xlsx.read(bytes)
                except Exception as e:
                    print(f"{e.__class__.__name__}: {e}")
                    continue

                if doc:
                    for sh in doc.sheets:
                        print(f"sheet: {sh.name}")
                        for i, row in enumerate(sh.get_rows()):
                            print(f"row {i}: {row}")
                            if i > 10:
                                print("...")
                                break


if __name__ == "__main__":
    main()
