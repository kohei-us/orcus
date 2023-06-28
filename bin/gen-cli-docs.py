#!/usr/bin/env python3

import argparse
import subprocess
import io
import os
import copy
from pathlib import Path


def _print_option(writable, option, description):
    line = ' '.join(option)
    print(f"- ``{line}``", file=writable)
    print(file=writable)

    line_buf = list()
    description_cleaned = list()
    for line in description:
        if not line_buf:
            line_buf = copy.deepcopy(line)
            continue

        if not line:
            # empty line
            if line_buf:
                description_cleaned.append(line_buf)
            description_cleaned.append(line)
            line_buf = list()
            continue

        if line[0] == '*':
            description_cleaned.append(line_buf)
            line_buf = list()

        line_buf.extend(line)

    if line_buf:
        description_cleaned.append(line_buf)

    for line in description_cleaned:
        if line and line[0] == '*':
            line[0] = '-'
            line = "  " + ' '.join(line)
            print(f"  {line}", file=writable)
            continue

        print("  " + ' '.join(line), file=writable)

    print(file=writable)


def _parse_and_print(writable, cmd_name, lines):
    print(cmd_name, file=writable)
    print('=' * len(cmd_name), file=writable)
    print(file=writable)

    # Usage (1st line)
    line = lines[0]
    prefix = "Usage: "
    if not line.startswith(prefix):
        raise RuntimeError("invalid output")

    cmd = line[len(prefix):]
    print("Usage", file=writable)
    print("-----", file=writable)
    print(file=writable)
    print(".. code-block::", file=writable)
    print(file=writable)
    print(f"   {cmd}", file=writable)
    print(file=writable)

    # Description (sentence block right below usage)
    for i, line in enumerate(lines[2:]):
        if line == "Options:":
            break
        print(line, file=writable)

    # Allowed option title
    lineno = i + 2
    line = lines[lineno]
    if not line or line[-1] != ':':
        print(lines)
        raise RuntimeError("invalid section title")

    line = line[:-1]
    print(line, file=writable)
    print('-' * len(line), file=writable)
    print(file=writable)

    # Options
    lineno += 1

    # determine the first indent length from the first line.
    indent = 0
    while lines[lineno][indent] == ' ':
        indent += 1

    # determine the column position of the option description.
    desc_pos = lines[lineno].find("Print this help.")
    if desc_pos < 0:
        raise RuntimeError("failed to parse the --help option line.")

    option_buf = list()
    desc_buf = list()
    for line in lines[lineno:]:
        if not line:
            continue

        option_s = line[indent:desc_pos]
        desc_s = line[desc_pos:]

        if option_s and option_s[0] == '-':
            # start a new option. if the current buffer is not empty, flush it first.
            if option_buf:
                _print_option(writable, option_buf, desc_buf)

            option_buf = option_s.split()
            desc_buf = list()

        desc_buf.append(desc_s.split())

    if option_buf:
        _print_option(writable, option_buf, desc_buf)


def parse(cmd_dir, output_dir, cmd_name):
    os.makedirs(output_dir, exist_ok=True)
    cmd_path = cmd_dir / cmd_name
    s = cmd_name.replace('-', '_')
    output_path = output_dir / f"{s}.rst"
    if not cmd_path.is_file():
        raise RuntimeError(f"command not found: {cmd_path}")

    output = subprocess.run([cmd_path, "-h"], stdout=subprocess.PIPE).stdout
    output = output.decode("utf-8")
    lines = output.split('\n')

    with open(output_path, "w") as f:
        _parse_and_print(f, cmd_name, lines)


def main():
    parser = argparse.ArgumentParser(
        description="Parse the output from the cli help, and convert it to rst output.")
    parser.add_argument(
        "--cmd-dir", "-c", type=Path, required=True,
        help="path to the directory where the orcus commands are.")
    parser.add_argument(
        "--output-dir", "-o", type=Path, required=True,
        help="path to the output directory.")
    args = parser.parse_args()

    if not args.cmd_dir.is_dir():
        raise RuntimeError(f"invalid command directory: {args.cmd_dir}")

    cmds = (
        "orcus-csv",
        "orcus-ods",
        "orcus-xlsx",
        "orcus-gnumeric",
        "orcus-json",
        "orcus-xml",
        "orcus-xls-xml",
        "orcus-yaml",
        "orcus-parquet",
    )
    for cmd in cmds:
        parse(args.cmd_dir, args.output_dir, cmd)


if __name__ == "__main__":
    main()

