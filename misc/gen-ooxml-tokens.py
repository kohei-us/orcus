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
import enum
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
    zip = zipfile.ZipFile(fpath, 'r')
    tokens = {}
    for item in zip.namelist():
        fd = zip.open(item, 'r')
        parser = XMLParser(fd.read())
        fd.close()
        parser.parse()
        for token in parser.tokens:
            tokens[token] = True
    zip.close()

    return sorted(tokens.keys())


class SchemaType(enum.Enum):
    OOXML = "ooxml"
    OPC = "opc"


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "-t", "--schema-type", default=SchemaType.OOXML, type=SchemaType,
        help="Specify the schema type.  Possible values are: 'ooxml', or 'opc'.  The default value is 'ooxml'.")
    parser.add_argument(
        "-i", "--input", required=True, type=str,
        help="Zip file containing schemas.")
    parser.add_argument(
        "constant_file", metavar="CONSTANT-FILE", nargs=1, type=argparse.FileType("w"),
        help="Output file to store constant values.")
    parser.add_argument(
        "name_file", metavar="NAME-FILE", nargs=1, type=argparse.FileType("w"),
        help="Output file to store constant string names.")
    args = parser.parse_args()

    tokens = get_all_tokens_from_zip(args.input)
    token_util.gen_token_constants(args.constant_file[0], tokens)
    token_util.gen_token_names(args.name_file[0], tokens)


if __name__ == '__main__':
    main()
