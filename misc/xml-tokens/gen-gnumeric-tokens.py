#!/usr/bin/env python
#************************************************************************
#
#  Copyright (c) 2010-2012 Kohei Yoshida
#
#  Permission is hereby granted, free of charge, to any person
#  obtaining a copy of this software and associated documentation
#  files (the "Software"), to deal in the Software without
#  restriction, including without limitation the rights to use,
#  copy, modify, merge, publish, distribute, sublicense, and/or sell
#  copies of the Software, and to permit persons to whom the
#  Software is furnished to do so, subject to the following
#  conditions:
#
#  The above copyright notice and this permission notice shall be
#  included in all copies or substantial portions of the Software.
#
#  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
#  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
#  OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
#  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
#  HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
#  WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
#  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
#  OTHER DEALINGS IN THE SOFTWARE.
#
#***********************************************************************

import xml.parsers.expat, sys
import token_util
import argparse
from pathlib import Path


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


def gen_tokens_from_schema(schema_file, extra_tokens):
    chars = schema_file.read_text()

    parser = XMLParser(chars)
    parser.parse()
    tokens = {}
    for token in parser.tokens:
        tokens[token] = True
    keys = list(tokens.keys())
    keys.extend(extra_tokens)
    keys = set(keys)  # remove potential duplicates
    keys = sorted(keys)
    return keys


def main ():
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--schema", type=Path, required=True,
        help="Path to Gnumeric's schema file, located in its repository and is named 'gnumeric.xsd'."
    )
    parser.add_argument(
        "--extra-tokens", type=Path, default="./gnumeric-extra-tokens.txt",
        help="File containing extra tokens to add with one token per line."
    )
    parser.add_argument(
        "--output-token-constants", type=Path, default="../../src/liborcus/gnumeric_token_constants.inl",
        help="Path to output file where token consant values are to be written."
    )
    parser.add_argument(
        "--output-tokens", type=Path, default="../../src/liborcus/gnumeric_tokens.inl",
        help="Path to output file where token consant values are to be written."
    )
    args = parser.parse_args()

    extra_tokens = args.extra_tokens.read_text().strip().split('\n')
    tokens = gen_tokens_from_schema(args.schema, extra_tokens)
    token_util.gen_token_constants(args.output_token_constants, tokens)
    token_util.gen_token_names(args.output_tokens, tokens)


if __name__ == '__main__':
    main()
