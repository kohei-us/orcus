#!/usr/bin/env python3
########################################################################
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
########################################################################

import argparse
import os
import os.path
import sys
import string
import pathlib
import enum
import importlib.util

import orcus


FILEEXT_GOOD = "orcus-pf.good"
FILEEXT_BAD = "orcus-pf.bad"
FILEEXT_OUT = "orcus-pf.out"


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
        if format_type == orcus.FormatType.ODS:
            from orcus import ods
            doc = ods.read(bytes)
        elif format_type == orcus.FormatType.XLSX:
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


def print_results(inpath):
    outpath = f"{inpath}.{FILEEXT_OUT}"
    with open(outpath, "r") as f:
        print()
        for line in f.readlines():
            print(f"  {line.strip()}")
        print()


def remove_result_files(rootdir):
    for root, dir, files in os.walk(rootdir):
        for filename in files:
            if filename.endswith(FILEEXT_OUT) or filename.endswith(FILEEXT_GOOD) or filename.endswith(FILEEXT_BAD):
                filepath = os.path.join(root, filename)
                os.remove(filepath)


def show_result_stats(rootdir):
    counts = dict(good=0, bad=0, skipped=0, unprocessed=0)
    for root, dir, files in os.walk(rootdir):
        for filename in files:
            if filename.endswith(FILEEXT_OUT) or filename.endswith(FILEEXT_GOOD) or filename.endswith(FILEEXT_BAD):
                continue

            inpath = os.path.join(root, filename)
            out_filepath = f"{inpath}.{FILEEXT_OUT}"
            good_filepath = f"{inpath}.{FILEEXT_GOOD}"
            bad_filepath = f"{inpath}.{FILEEXT_BAD}"
            if os.path.isfile(good_filepath):
                counts["good"] += 1
            elif os.path.isfile(bad_filepath):
                counts["bad"] += 1
            elif os.path.isfile(out_filepath):
                counts["skipped"] += 1
            else:
                counts["unprocessed"] += 1

    print("* result counts")
    for cat in ("good", "bad", "skipped", "unprocessed"):
        print(f"  * {cat}: {counts[cat]}")

    total = counts["good"] + counts["bad"]
    if total:
        print("* ratios")
        print(f"  * good: {counts['good']/total*100:.1f}%")
        print(f"  * bad: {counts['bad']/total*100:.1f}%")


def show_results(rootdir, good, bad):
    for root, dir, files in os.walk(rootdir):
        for filename in files:
            if filename.endswith(FILEEXT_OUT) or filename.endswith(FILEEXT_GOOD) or filename.endswith(FILEEXT_BAD):
                continue
            inpath = os.path.join(root, filename)
            good_filepath = f"{inpath}.{FILEEXT_GOOD}"
            bad_filepath = f"{inpath}.{FILEEXT_BAD}"

            if os.path.isfile(good_filepath) and good:
                print(sanitize_string(inpath), flush=True)
                print_results(inpath)
            elif os.path.isfile(bad_filepath) and bad:
                print(sanitize_string(inpath), flush=True)
                print_results(inpath)
            else:
                continue


def load_module_from_filepath(filepath):
    if not os.path.isfile(filepath):
        raise RuntimeError(f"{filepath} is not a valid file.")

    mod_name = os.path.splitext(os.path.basename(filepath))[0]
    spec = importlib.util.spec_from_file_location(mod_name, filepath)
    mod = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(mod)
    return mod


def main():
    parser = argparse.ArgumentParser(
        description="""This script allows you to load a collection of input files
        for testing purpuses.""")
    parser.add_argument("-p", "--processor", type=str, help="Python module file containing callback functions.")
    parser.add_argument(
        "--remove-results", action="store_true", default=False,
        help="Remove all cached results files from the directory tree.")
    parser.add_argument(
        "--results", action="store_true", default=False,
        help="Display the results of the processed files.")
    parser.add_argument(
        "--good", action="store_true", default=False,
        help="Display the results of the successfully processed files.")
    parser.add_argument(
        "--bad", action="store_true", default=False,
        help="Display the results of the unsuccessfully processed files.")
    parser.add_argument(
        "--stats", action="store_true", default=False,
        help="Display statistics of the results.  Use it with --results.")
    parser.add_argument(
        "rootdir", metavar="ROOT-DIR",
        help="Root directory below which to recursively find and process test files.")
    args = parser.parse_args()

    if args.remove_results:
        remove_result_files(args.rootdir)
        return

    if args.results:
        if args.stats:
            show_result_stats(args.rootdir)
            return

        show_results(args.rootdir, args.good, args.bad)
        return

    mod = load_module_from_filepath(args.processor) if args.processor else None

    file_count = 0
    for root, dir, files in os.walk(args.rootdir):
        for filename in files:
            if filename.endswith(FILEEXT_OUT) or filename.endswith(FILEEXT_GOOD) or filename.endswith(FILEEXT_BAD):
                continue

            inpath = os.path.join(root, filename)
            outpath = f"{inpath}.{FILEEXT_OUT}"
            print(file_count, sanitize_string(inpath), flush=True)
            file_count += 1
            buf = list()

            good_filepath = f"{inpath}.{FILEEXT_GOOD}"
            bad_filepath = f"{inpath}.{FILEEXT_BAD}"

            if os.path.isfile(good_filepath) or os.path.isfile(bad_filepath):
                print("already processed. skipping...")
                continue

            success = False
            with open(inpath, 'rb') as f:
                bytes = f.read()

            doc, status, output = load_doc(bytes)
            buf.extend(output)
            if doc and mod:
                buf.extend(mod.process_document(inpath, doc))

            with open(outpath, "w") as f:
                f.write("\n".join(buf))
            print("\n".join(buf))

            if status == LoadStatus.SUCCESS:
                pathlib.Path(good_filepath).touch()
            elif status == LoadStatus.FAILURE:
                pathlib.Path(bad_filepath).touch()


if __name__ == "__main__":
    main()
