#!/usr/bin/env python
########################################################################
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
########################################################################

import xml.parsers.expat
import sys
import argparse
from pathlib import Path

import token_util


NS_RNG = "http://relaxng.org/ns/structure/1.0"


class TokenParser:

    def __init__ (self, strm):
        self.__strm = strm
        self.__elem = None
        self.tokens = set()

    def start_element(self, name, attrs):
        self.__elem = name
        if name in {f"{NS_RNG}:element", f"{NS_RNG}:attribute"} and "name" in attrs:
            tokens = attrs['name'].split(':')
            n = len(tokens)
            if n == 1:
                # namespace-less token
                self.tokens.add(tokens[0])
            elif n == 2:
                # namespaced token
                self.tokens.add(tokens[1])
            else:
                sys.stderr.write("unrecognized token type: "+attrs['name'])
                sys.exit(1)

            for token in tokens:
                self.tokens.add(token)

    def character(self, data):
        if self.__elem == f"{NS_RNG}:value":
            s = data.strip()
            if len(s) > 0:
                self.tokens.add(s)

    def parse(self):
        p = xml.parsers.expat.ParserCreate(encoding="utf-8", namespace_separator=":")
        p.StartElementHandler = self.start_element
        p.CharacterDataHandler = self.character
        p.Parse(self.__strm, 1)

        self.tokens = sorted(self.tokens)


class NSParser:

    def __init__ (self, strm):
        self.__strm = strm
        self.__elem = None
        self.ns_values = dict()  # namespace values

    def start_element(self, name, attrs):
        self.__elem = name
        if name.endswith("grammar"):
            names = attrs.keys()
            for name in names:
                tokens = name.split(':')
                if len(tokens) < 2 or tokens[0] != "xmlns":
                    continue

                val = attrs[name]
                self.ns_values[tokens[1]] = val

    def parse(self):
        p = xml.parsers.expat.ParserCreate(encoding="utf-8")
        p.StartElementHandler = self.start_element
        p.Parse(self.__strm, 1)

        ns_values = list()
        for k, v in self.ns_values.items():
            ns_values.append((k, v))

        self.ns_values = sorted(ns_values, key=lambda x: x[0])


def gen_namespace_tokens(filepath, ns_values):

    # header (.hpp)
    filepath_hpp = filepath + "_hpp.inl"
    outfile = open(filepath_hpp, 'w')
    outfile.write("namespace orcus {\n\n")
    for key, _ in ns_values:
        outfile.write("extern const xmlns_id_t NS_odf_")
        outfile.write(key)
        outfile.write(";\n")
    outfile.write("\nextern const xmlns_id_t* NS_odf_all;\n")
    outfile.write("\n}\n\n")
    outfile.close()

    # source (.cpp)
    filepath_cpp = filepath + "_cpp.inl"
    outfile = open(filepath_cpp, 'w')
    outfile.write("namespace orcus {\n\n")
    for key, value in ns_values:
        outfile.write("const xmlns_id_t NS_odf_")
        outfile.write(key)
        outfile.write(" = \"")
        outfile.write(value)
        outfile.write("\"")
        outfile.write(";\n")

    outfile.write("\n")
    outfile.write("namespace {\n\n")
    outfile.write("const xmlns_id_t odf_ns[] = {\n")
    for key, _ in ns_values:
        outfile.write("    NS_odf_")
        outfile.write(key)
        outfile.write(",\n")
    outfile.write("    nullptr\n")
    outfile.write("};\n\n")
    outfile.write("} // anonymous\n\n")

    outfile.write("const xmlns_id_t* NS_odf_all = odf_ns;\n\n")

    outfile.write("}\n\n")
    outfile.close()


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--ns-file-prefix", type=str,
        help="file name prefix for optioal namespace constant files")
    parser.add_argument(
        "--summary-output", type=Path,
        help="optional output file to write collected token data summary")
    parser.add_argument(
        "--token-constants", type=Path,
        help="path to C++ output file where token consants are to be written to")
    parser.add_argument(
        "--token-names", type=Path,
        help="path to C++ output file where token names are to be written to")
    parser.add_argument(
        "odf_schema", metavar="ODF-SCHEMA", type=Path, help="path to RNG ODF schema file")
    args = parser.parse_args()

    if not args.odf_schema.is_file():
        print(f"{args.odf_schema} is not a valid file.", file=sys.stderr)
        sys.exit(1)

    schema_content = args.odf_schema.read_text()
    parser = TokenParser(schema_content)
    parser.parse()
    tokens = parser.tokens

    parser = NSParser(schema_content)
    parser.parse()
    ns_values = parser.ns_values

    if args.summary_output:
        summary_content_buf = list()
        summary_content_buf.append("list of tokens:")

        for token in tokens:
            summary_content_buf.append(f"- \"{token}\"")

        summary_content_buf.append("list of namespaces:")

        for ns, value in ns_values:
            summary_content_buf.append(f"- {ns}: \"{value}\"")

        args.summary_output.write_text("\n".join(summary_content_buf))

    if args.token_constants:
        with open(args.token_constants, "w") as f:
            token_util.gen_token_constants(f, tokens)

    if args.token_names:
        with open(args.token_names, "w") as f:
            token_util.gen_token_names(f, tokens)

    if args.ns_file_prefix is not None:
        gen_namespace_tokens(args.ns_file_prefix, ns_values)


if __name__ == '__main__':
    main()
