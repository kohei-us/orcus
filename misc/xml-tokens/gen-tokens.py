#!/usr/bin/env python3
########################################################################
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
########################################################################

import token_util
import argparse
import sys
from pathlib import Path


desc = """Generate C++ source files from a list of tokens.

To generate tokens files for Excel 2003 XML (xls-xml), run

  %(prog)s xls-xml-tokens.txt \\
    ../../src/liborcus/xls_xml_token_constants.inl \\
    ../../src/liborcus/xls_xml_tokens.inl \\
"""

def main ():
    parser = argparse.ArgumentParser(
        description=desc,
        formatter_class=argparse.RawTextHelpFormatter
    )
    parser.add_argument("tokenlist", type=Path, help="plain-text file that contains a list of tokens.")
    parser.add_argument("output1", type=Path, help="output file that will contain XML token values.")
    parser.add_argument("output2", type=Path, help="output file that will contain XML token names.")
    args = parser.parse_args()

    tokens = {}
    with open(args.tokenlist, "r") as f:
        for line in f.readlines():
            token = line.strip()
            tokens[token] = True

    tokens = sorted(tokens.keys())
    token_util.gen_token_constants(args.output1, tokens)
    token_util.gen_token_names(args.output2, tokens)


if __name__ == "__main__":
    main()
