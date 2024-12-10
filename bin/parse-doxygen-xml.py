#!/usr/bin/env python
########################################################################
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
########################################################################

import sys
import xml.etree.ElementTree as ET
import argparse
from pathlib import Path


def list_kinds(rootdir):

    compond_kinds = set()
    member_kinds = set()

    for p in rootdir.iterdir():
        tree = ET.parse(p)
        root = tree.getroot()

        for elem in root.findall(".//compounddef"):
            kind = elem.get("kind")
            if kind:
                compond_kinds.add(kind)

        for elem in root.findall(".//memberdef"):
            kind = elem.get("kind")
            if kind:
                member_kinds.add(kind)

    print("compound kinds:")
    for k in compond_kinds:
        print(f"  - {k}")

    print("member kinds:")
    for k in member_kinds:
        print(f"  - {k}")


class SymbolTree:

    def __init__(self):
        self._root = dict()

    def add(self, ns, type, name):
        ns_parts = ns.split("::")
        node = self._root
        for part in ns_parts:
            if part not in node:
                node[part] = dict()
            node = node[part]

        if "_store" not in node:
            node["_store"] = dict()

        store = node["_store"]
        if type not in store:
            store[type] = list()
        store[type].append(name)

    def _dump_recurse(self, node):
        for scope, child in node.items():
            if scope == "detail":
                continue
            if scope == "_store":
                ns = "::".join(self._scope)
                self._symbols[ns] = child
                continue
            self._scope.append(scope)
            self._dump_recurse(child)
            self._scope.pop()

    def dump(self):
        from pprint import pprint
        self._scope = []
        self._symbols = dict()
        self._dump_recurse(self._root)
        all_scopes = sorted(self._symbols.keys())
        for scope in all_scopes:
            print(f"namespace: {scope}")
            symbols = self._symbols[scope]
            if "enum" in symbols:
                print("  enum:")
                for symbol in symbols["enum"]:
                    print(f"    - {symbol}")

            if "typedef" in symbols:
                print("  typedef:")
                for symbol in symbols["typedef"]:
                    print(f"    - {symbol}")

            if "variable" in symbols:
                print("  variable:")
                for symbol in symbols["variable"]:
                    print(f"    - {symbol}")

            if "function" in symbols:
                print("  function:")
                for symbol in symbols["function"]:
                    print(f"    - {symbol}")

            if "class" in symbols:
                print("  class:")
                for symbol in symbols["class"]:
                    print(f"    - {symbol}")

            if "struct" in symbols:
                print("  struct:")
                for symbol in symbols["struct"]:
                    print(f"    - {symbol}")


def dump_symbols(rootdir):

    all_symbols = list()
    type_scope = set()

    for p in rootdir.iterdir():
        tree = ET.parse(p)
        root = tree.getroot()

        for ns_elem in root.findall(".//compounddef[@kind='namespace']"):
            ns_name = ns_elem.findtext("compoundname")
            for elem in ns_elem.findall(".//memberdef[@kind='enum']"):
                name = elem.findtext("name")
                all_symbols.append((ns_name, "enum", name))

            for elem in ns_elem.findall(".//memberdef[@kind='typedef']"):
                name = elem.findtext("name")
                all_symbols.append((ns_name, "typedef", name))

            for elem in ns_elem.findall(".//memberdef[@kind='function']"):
                name = elem.findtext("name")
                if name.startswith("operator"):
                    continue
                all_symbols.append((ns_name, "function", name))

            for elem in ns_elem.findall(".//memberdef[@kind='variable']"):
                name = elem.findtext("name")
                all_symbols.append((ns_name, "variable", name))

        for elem in root.findall(".//compounddef[@kind='class']"):
            name = elem.findtext("compoundname")
            ns, name = name.rsplit('::', maxsplit=1)
            all_symbols.append((ns, "class", name))

            type_scope.add(f"{ns}::{name}")

        for elem in root.findall(".//compounddef[@kind='struct']"):
            name = elem.findtext("compoundname")
            ns, name = name.rsplit('::', maxsplit=1)
            all_symbols.append((ns, "struct", name))

            type_scope.add(f"{ns}::{name}")

    def _to_key(x):
        return f"{x[0]}:{x[1]}:{x[2]}"

    symbol_tree = SymbolTree()

    for scope, type, name in sorted(all_symbols, key=_to_key):
        if scope in type_scope:
            # this is a nested inner type - skip it
            continue

        symbol_tree.add(scope, type, name)

    symbol_tree.dump()


def main():
    parser = argparse.ArgumentParser(description="Paser and analyze XML outputs from doxygen.")
    parser.add_argument("--mode", type=str, required=True, help="Type of action to perform.")
    parser.add_argument("rootdir", type=Path)
    args = parser.parse_args()

    if args.mode == "kinds":
        list_kinds(args.rootdir)
    elif args.mode == "symbols":
        dump_symbols(args.rootdir)
    else:
        print(f"unknown mode: {args.mode}", file=sys.stderr)


if __name__ == "__main__":
    main()
