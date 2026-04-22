#!/usr/bin/env python3
########################################################################
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
########################################################################
"""Normalize the formatting of source files.

Ensures consistent whitespace around the license block, #pragma once, and vim
modeline in .hpp files, and strips trailing whitespace from all file types.
"""

import argparse
import sys
from pathlib import Path


def _strip_file_ending(path):
    data = path.read_bytes().rstrip(b" \t\r\n") + b"\n"
    path.write_bytes(data)


def _normalize_hpp(path):
    lines = path.read_text(encoding="utf-8").splitlines(keepends=True)

    # Find end of license block
    license_end = None
    for i, l in enumerate(lines):
        if l.rstrip("\r\n") == " */":
            license_end = i
            break

    if license_end is None:
        raise RuntimeError(f"{path}: end of license block not detected")

    # Find #pragma once
    pragma_pos = None
    for i, l in enumerate(lines):
        if l.strip() == "#pragma once":
            pragma_pos = i
            break

    new_lines = list(lines[:license_end + 1])

    if pragma_pos is None:
        raise RuntimeError(f"{path}: missing #pragma once")

    new_lines += ["\n", lines[pragma_pos]]
    rest_start = pragma_pos + 1

    # Skip blank lines after #pragma once
    while rest_start < len(lines) and not lines[rest_start].strip():
        rest_start += 1

    new_lines += ["\n"] + lines[rest_start:]

    # Normalize blank lines around vim modeline
    vim_pos = None
    for i, l in enumerate(new_lines):
        if l.startswith("/* vim:"):
            vim_pos = i
            break

    if vim_pos is None:
        raise RuntimeError(f"{path}: vim modeline not found")

    # find the last non-empty line above the vim modeline
    code_pos = vim_pos - 1
    while code_pos >= 0 and not new_lines[code_pos].strip():
        code_pos -= 1
    new_lines = new_lines[:code_pos + 1] + ["\n", new_lines[vim_pos]]

    if new_lines != lines:
        path.write_text("".join(new_lines), encoding="utf-8")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("path", type=Path, nargs="*", help="source file to normalize")
    args = parser.parse_args()

    try:
        for path in args.path:
            if path.stat().st_size == 0:
                continue  # skip empty file

            _strip_file_ending(path)

            if path.suffix == ".hpp":
                _normalize_hpp(path)

    except Exception as e:
        print(e, file=sys.stderr)
        sys.exit(1)


if __name__ == "__main__":
    main()

