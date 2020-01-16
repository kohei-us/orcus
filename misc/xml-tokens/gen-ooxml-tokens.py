#!/usr/bin/env python3
########################################################################
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
########################################################################

import xml.parsers.expat
import zipfile
import argparse
import sys
import token_util


class XMLParser:

    def __init__ (self, strm):
        self.__strm = strm
        self.__elem = None
        self.tokens = []

    def start_element(self, name, attrs):
        self.__elem = name
        if name in ['xs:element', 'xs:attribute', 'xsd:element', 'xsd:attribute'] and "name" in attrs:
            token = attrs['name']
            if len(token) > 0:
                self.tokens.append(token)

    def end_element(self, name):
        pass

    def character(self, data):
        pass

    def parse (self):
        p = xml.parsers.expat.ParserCreate()
        p.StartElementHandler = self.start_element
        p.EndElementHandler = self.end_element
        p.CharacterDataHandler = self.character
        p.Parse(self.__strm, 1)


def get_all_tokens_from_zip(fpath):
    with zipfile.ZipFile(fpath, 'r') as zip:
        tokens = set()
        for item in zip.namelist():
            fd = zip.open(item, 'r')
            parser = XMLParser(fd.read())
            fd.close()
            parser.parse()
            tokens.update(parser.tokens)

    return tokens


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "-i", "--input", required=True, type=str,
        help="Zip file containing schemas.")
    parser.add_argument("--extra-input", type=argparse.FileType("r"), help="Optional input file containing extra token names.")
    parser.add_argument(
        "constant_file", metavar="CONSTANT-FILE", nargs=1, type=argparse.FileType("w"),
        help="Output file to store constant values.")
    parser.add_argument(
        "name_file", metavar="NAME-FILE", nargs=1, type=argparse.FileType("w"),
        help="Output file to store constant string names.")
    args = parser.parse_args()

    tokens = get_all_tokens_from_zip(args.input)

    if args.extra_input:
        extra_tokens = [x.strip() for x in args.extra_input.readlines()]
        tokens.update(extra_tokens)

    tokens = sorted(list(tokens))
    token_util.gen_token_constants(args.constant_file[0], tokens)
    token_util.gen_token_names(args.name_file[0], tokens)


if __name__ == '__main__':
    main()
