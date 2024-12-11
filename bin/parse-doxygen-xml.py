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

    def walk(self, func):
        self._scope = list()
        self._walk(self._root, 0, func)

    def _walk(self, node, level, func):
        indent = " " * level * 2
        keys = sorted(node.keys())
        has_symbols = "_store" in keys
        if has_symbols:
            symbols = node["_store"]
            keys.remove("_store")
            func(self._scope, keys, symbols)
        else:
            func(self._scope, keys, {})

        for key in keys:
            child = node[key]
            self._scope.append(key)
            self._walk(child, level + 1, func)
            self._scope.pop()


def build_symbol_tree(rootdir):
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

    return symbol_tree


def dump_symbols(rootdir):
    symbol_tree = build_symbol_tree(rootdir)
    symbol_tree.dump()


def generate_rst(scope, child_scopes, symbols):
    if scope:
        ns = "::".join(scope)
        title = "namespace " + ns
    else:
        title = "C++ API Reference"

    buf = [
        "",
        title,
        '=' * len(title),
        "",
    ]

    if "enum" in symbols:
        this_buf = [
            "Enum",
            "----",
            ""
        ]

        for name in symbols["enum"]:
            full_name = f"{ns}::{name}"
            block = [
                name,
                "^" * len(name),
                f".. doxygenenum:: {full_name}",
                "",
            ]
            this_buf.extend(block)

        this_buf.append("")
        buf.extend(this_buf)

    if "typedef" in symbols:
        this_buf = [
            "Type aliases",
            "------------",
            ""
        ]

        for name in symbols["typedef"]:
            full_name = f"{ns}::{name}"
            block = [
                name,
                "^" * len(name),
                f".. doxygentypedef:: {full_name}",
                "",
            ]
            this_buf.extend(block)

        this_buf.append("")
        buf.extend(this_buf)

    if "variable" in symbols:
        this_buf = [
            "Constants",
            "---------",
            ""
        ]

        for name in symbols["variable"]:
            full_name = f"{ns}::{name}"
            block = [
                name,
                "^" * len(name),
                f".. doxygenvariable:: {full_name}",
                "",
            ]
            this_buf.extend(block)

        this_buf.append("")
        buf.extend(this_buf)

    if "function" in symbols:
        this_buf = [
            "Functions",
            "---------",
            ""
        ]

        for name in symbols["function"]:
            full_name = f"{ns}::{name}"
            block = [
                f"{name}",
                "^" * len(name),
                f".. doxygenfunction:: {full_name}",
                "",
            ]
            this_buf.extend(block)

        this_buf.append("")
        buf.extend(this_buf)

    if "struct" in symbols:
        this_buf = [
            "Struct",
            "------",
            ""
        ]

        for name in symbols["struct"]:
            full_name = f"{ns}::{name}"
            block = [
                name,
                "^" * len(name),
                f".. doxygenstruct:: {full_name}",
                f"   :members:",
                "",
            ]
            this_buf.extend(block)

        this_buf.append("")
        buf.extend(this_buf)

    if "class" in symbols:
        this_buf = [
            "Classes",
            "-------",
            ""
        ]

        for name in symbols["class"]:
            full_name = f"{ns}::{name}"
            block = [
                name,
                "^" * len(name),
                f".. doxygenclass:: {full_name}",
                f"   :members:",
                "",
            ]
            this_buf.extend(block)

        this_buf.append("")
        buf.extend(this_buf)

    if child_scopes:
        toctree = [
            "Child namespaces",
            "----------------",
            ""
            ".. toctree::",
            "   :maxdepth: 1",
            "",
        ]

        for cs in child_scopes:
            p = f"{cs}/index.rst"
            toctree.append(f"   {p}")

        buf.extend(toctree)

    return '\n'.join(buf)


def generate_doctree(rootdir, outdir):
    symbol_tree = build_symbol_tree(rootdir)

    def _func(scope, child_scopes, symbols):
        thisdir = outdir.joinpath(*scope)
        index_file = thisdir / "index.rst"
        print(index_file)
        thisdir.mkdir(parents=True, exist_ok=True)
        index_file.write_text(generate_rst(scope, child_scopes, symbols))

    symbol_tree.walk(_func)


def main():
    parser = argparse.ArgumentParser(
        description="Parse doxygen XML outputs and perform actions.",
        epilog="""To re-generate the C++ API Reference, run this script with
        --mode doctree and specify the output directory to ./doc/cpp.
        """
    )
    parser.add_argument("--mode", type=str, required=True, help="Type of action to perform.")
    parser.add_argument("--output", "-o", type=Path, help="Output directory path.")
    parser.add_argument("rootdir", type=Path, help="Directory where the doxygen XML files are found.")
    args = parser.parse_args()

    if args.mode == "kinds":
        list_kinds(args.rootdir)
    elif args.mode == "symbols":
        dump_symbols(args.rootdir)
    elif args.mode == "doctree":
        generate_doctree(args.rootdir, args.output)
    else:
        print(f"unknown mode: {args.mode}", file=sys.stderr)


if __name__ == "__main__":
    main()
