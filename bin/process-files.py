#!/usr/bin/env python3

import argparse
import os
import os.path
import sys
import string


def sanitize_string(s):
    """Replace non-printable characters with \x[value]."""

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

    for root, dir, files in os.walk(args.rootdir):
        for filename in files:
            filepath = os.path.join(root, filename)
            print(sanitize_string(filepath), flush=True)
            with open(filepath, 'rb') as f:
                bytes = f.read()
                print(f"* {len(bytes)} bytes")


if __name__ == "__main__":
    main()
