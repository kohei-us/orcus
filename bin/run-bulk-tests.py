#!/usr/bin/env python3
########################################################################
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
########################################################################

import argparse
import io
import os
import subprocess
from pathlib import Path


def main():
    desc="""Tool to bulk-test a number of files and collect all their results
    into a single output directory."""

    parser = argparse.ArgumentParser(description=desc)
    parser.add_argument("--args", type=str, required=True)
    parser.add_argument("--ext", "-e", type=str, required=True, help="File extension to use.")
    parser.add_argument("--outdir", "-o", type=Path, required=True, help="Output directory to put the run results in.")
    parser.add_argument("dirs", nargs='*', type=Path)
    args = parser.parse_args()

    args.outdir.joinpath("good").mkdir(parents=True, exist_ok=True)
    args.outdir.joinpath("bad").mkdir(parents=True, exist_ok=True)

    ext = args.ext
    if ext and ext[0] != '.':
        ext = f".{ext}"

    count = 0
    for parent_dir in args.dirs:
        for rootdir, _, files in os.walk(parent_dir):
            for f in files:
                filepath = Path(rootdir) / f
                if filepath.suffix != ext:
                    continue

                buf = io.StringIO()
                cmd = args.args.split()
                cmd.append(str(filepath))
                res = subprocess.run(cmd, capture_output=True)
                outpath = args.outdir
                outpath /= "good" if res.returncode == 0 else "bad"
                outpath /= f"{count:04d}.txt"
                print("-- command --", file=buf)
                print(" ".join(cmd), file=buf)
                print("-- return-code --", file=buf)
                print(res.returncode, file=buf)
                print("-- stdout --", file=buf)
                print(res.stdout.decode("utf-8"), file=buf)
                print("-- stderr --", file=buf)
                print(res.stderr.decode("utf-8"), file=buf)
                outpath.write_text(buf.getvalue())
                count += 1


if __name__ == "__main__":
    main()

