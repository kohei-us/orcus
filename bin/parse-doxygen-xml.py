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
import io
from dataclasses import dataclass, field
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


@dataclass
class SymbolProps:
    """Extra properties of a symbol."""
    location: str = field(default=str)


@dataclass
class FuncProps(SymbolProps):
    """Extra properties of a function symbol."""
    argsstring: str = None


@dataclass
class EnumProps(SymbolProps):
    """Extra properties of a enum symbol."""

    members: list[str] = field(default_factory=list)
    """List of enum members."""


class SymbolTree:
    """Tree of namespaces."""

    def __init__(self):
        self._root = dict()

    def add(self, ns, type, name, props):
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

        store[type].append((name, props))

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
        self._scope = []
        self._symbols = dict()
        self._dump_recurse(self._root)
        all_scopes = sorted(self._symbols.keys())
        for scope in all_scopes:
            print(f"namespace: {scope}")
            symbols = self._symbols[scope]
            if "enum" in symbols:
                print("  enum:")
                for symbol, props in symbols["enum"]:
                    print(f"    - name: {symbol}")
                    print(f"      location: {props.location}")
                    print(f"      members:")
                    for member in props.members:
                        print(f"        - {member}")

            if "typedef" in symbols:
                print("  typedef:")
                for symbol, props in symbols["typedef"]:
                    print(f"    - name: {symbol}")
                    print(f"      location: {props.location}")

            if "variable" in symbols:
                print("  variable:")
                for symbol, props in symbols["variable"]:
                    print(f"    - name: {symbol}")
                    print(f"      location: {props.location}")

            if "function" in symbols:
                print("  function:")
                for symbol, props in symbols["function"]:
                    print(f"    - name: {symbol}")
                    print(f"      argsstring: {props.argsstring}")
                    print(f"      location: {props.location}")

            if "class" in symbols:
                print("  class:")
                for symbol, props in symbols["class"]:
                    print(f"    - name: {symbol}")
                    print(f"      location: {props.location}")

            if "struct" in symbols:
                print("  struct:")
                for symbol, props in symbols["struct"]:
                    print(f"    - name: {symbol}")
                    print(f"      location: {props.location}")

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
                location = elem.find(".//location").attrib["file"]
                members = [elem_value.findtext("name") for elem_value in elem.findall(".//enumvalue")]
                props = EnumProps(members=members, location=location)
                all_symbols.append((ns_name, "enum", name, props))

            for elem in ns_elem.findall(".//memberdef[@kind='typedef']"):
                name = elem.findtext("name")
                location = elem.find(".//location").attrib["file"]
                props = SymbolProps(location=location)
                all_symbols.append((ns_name, "typedef", name, props))

            for elem in ns_elem.findall(".//memberdef[@kind='function']"):
                name = elem.findtext("name")
                if name.startswith("operator"):
                    continue
                location = elem.find(".//location").attrib["file"]
                props = FuncProps(location=location)
                props.argsstring = elem.findtext("argsstring")
                all_symbols.append((ns_name, "function", name, props))

            for elem in ns_elem.findall(".//memberdef[@kind='variable']"):
                name = elem.findtext("name")
                location = elem.find(".//location").attrib["file"]
                props = SymbolProps(location=location)
                all_symbols.append((ns_name, "variable", name, props))

        for elem in root.findall(".//compounddef[@kind='class']"):
            name = elem.findtext("compoundname")
            location = elem.find(".//location").attrib["file"]
            ns, name = name.rsplit('::', maxsplit=1)
            props = SymbolProps(location=location)
            all_symbols.append((ns, "class", name, props))

            type_scope.add(f"{ns}::{name}")

        for elem in root.findall(".//compounddef[@kind='struct']"):
            name = elem.findtext("compoundname")
            location = elem.find(".//location").attrib["file"]
            ns, name = name.rsplit('::', maxsplit=1)
            props = SymbolProps(location=location)
            all_symbols.append((ns, "struct", name, props))

            type_scope.add(f"{ns}::{name}")

    def _to_key(x):
        return f"{x[0]}:{x[1]}:{x[2]}"

    symbol_tree = SymbolTree()

    for scope, type, name, props in sorted(all_symbols, key=_to_key):
        if scope in type_scope:
            # this is a nested inner type - skip it
            continue

        symbol_tree.add(scope, type, name, props)

    return symbol_tree


def dump_symbols(rootdir):
    symbol_tree = build_symbol_tree(rootdir)
    symbol_tree.dump()


def find_parent_dir(path: Path, name: str):
    p = path
    while p.name != name:
        p = p.parent
    return p


def create_enum_stream_test(rootdir, output_dir):
    symbol_tree = build_symbol_tree(rootdir)

    output_dir.mkdir(parents=True, exist_ok=True)

    headers = set()
    func_bodies = dict()

    # grow this list
    included_symbols = set([
        "orcus::format_t",
        "orcus::spreadsheet::auto_filter_node_op_t",
        "orcus::spreadsheet::auto_filter_op_t",
        "orcus::spreadsheet::border_style_t",
        "orcus::spreadsheet::border_style_t",
        "orcus::spreadsheet::formula_grammar_t",
        "orcus::spreadsheet::hor_alignment_t",
        "orcus::spreadsheet::pivot_axis_t",
        "orcus::spreadsheet::pivot_cache_group_by_t",
        "orcus::spreadsheet::pivot_data_subtotal_t",
        "orcus::spreadsheet::pivot_data_show_data_as_t",
        "orcus::spreadsheet::pivot_field_item_t",
        "orcus::spreadsheet::strikethrough_style_t",
        "orcus::spreadsheet::strikethrough_text_t",
        "orcus::spreadsheet::strikethrough_type_t",
        "orcus::spreadsheet::strikethrough_width_t",
        "orcus::spreadsheet::underline_count_t",
        "orcus::spreadsheet::underline_spacing_t",
        "orcus::spreadsheet::underline_style_t",
        "orcus::spreadsheet::underline_thickness_t",
        "orcus::spreadsheet::ver_alignment_t",
    ])

    # either not worth testing or cannot be tested for reasons
    skipped_symbols = set([
        "orcus::string_escape_char_t",
        "orcus::spreadsheet::error_value_t", # non-standard labels
        "orcus::yaml::detail::keyword_t",
        "orcus::yaml::detail::parse_token_t",
        "orcus::yaml::detail::scope_t",
        "orcus::yaml::node_t",
    ])

    def _func(scope, b, symbols):
        ns = "::".join(scope)
        enums = symbols.get("enum", [])
        for enum, props in enums:
            type_symbol = f"{ns}::{enum}"
            if type_symbol in skipped_symbols:
                continue

            disabled = type_symbol not in included_symbols
            print(f"\u274c {type_symbol}" if disabled else f"\u2705 {type_symbol}")
            header_path = Path(props.location)
            include_dir = find_parent_dir(header_path, "include")
            header_path = header_path.relative_to(include_dir)
            headers.add(header_path)

            strm = io.StringIO()
            # generate unit test code
            def _print(*args):
                print(*args, file=strm)

            func_name = "test_" + type_symbol.replace("::", "_")
            _print(f"void {func_name}()")
            _print("{")
            if disabled:
                _print("#if 0")
            _print("    ORCUS_TEST_FUNC_SCOPE;")
            _print()
            for m in props.members:
                mem_type = f"{type_symbol}::{m}"
                mem_value = m.replace('_', '-')
                _print(f'    {{ bool result = verify_stream_value({mem_type}, "{mem_value}"); assert(result); }}')

            if disabled:
                _print("#endif")
            _print("}")

            strm.seek(0)
            func_bodies[func_name] = strm.getvalue()

    symbol_tree.walk(_func)

    test_file = output_dir / "enum_stream_test.cpp"
    with test_file.open("w") as f:
        def _print(*args):
            print(*args, file=f)
        _print(f"// auto-generated by {Path(__file__).name}")
        _print('#include "test_global.hpp"')
        for header in sorted(headers):
            _print(f"#include <{header}>")

        _print()
        _print("using orcus::test::verify_stream_value;")
        _print()
        for func_name in sorted(func_bodies.keys()):
            func_body = func_bodies[func_name]
            _print(func_body)

        _print()
        _print("int main()")
        _print("{")
        for func_name in sorted(func_bodies.keys()):
            _print(f"    {func_name}();")
        _print()
        _print("    return EXIT_SUCCESS;")
        _print("}")


def generate_rst(thisdir, scope, child_scopes, symbols):
    include_dir = Path(__file__).parent.parent / "include"

    if scope:
        ns = "::".join(scope)
        title = "namespace " + ns
        ref_anchor = "ns-" + '-'.join(scope)
    else:
        ref_anchor = "cpp-api"
        title = "C++ API Reference"

    buf = [
        f".. _{ref_anchor}:",
        "",
        title,
        '=' * len(title),
        "",
    ]

    if "enum" in symbols:
        this_buf = [
            "Enum",
            "----",
            "",
            ".. toctree::",
            "   :maxdepth: 1",
            "",
        ]

        for name, props in symbols["enum"]:
            child_file = f"enum-{name}.rst"
            this_buf.append(f"   {child_file}")

            header = Path(props.location).relative_to(include_dir)

            full_name = f"{ns}::{name}"
            block = [
                name,
                "=" * len(name),
                "",
                f"Defined in header: <{header}>",
                "",
                f".. doxygenenum:: {full_name}",
            ]

            thisdir.joinpath(child_file).write_text("\n".join(block))

        this_buf.append("")
        buf.extend(this_buf)

    if "typedef" in symbols:
        this_buf = [
            "Type aliases",
            "------------",
            "",
            ".. toctree::",
            "   :maxdepth: 1",
            "",
        ]

        for name, props in symbols["typedef"]:
            child_file = f"typedef-{name}.rst"
            this_buf.append(f"   {child_file}")

            header = Path(props.location).relative_to(include_dir)

            full_name = f"{ns}::{name}"
            block = [
                name,
                "=" * len(name),
                "",
                f"Defined in header: <{header}>",
                "",
                f".. doxygentypedef:: {full_name}",
            ]

            thisdir.joinpath(child_file).write_text("\n".join(block))

        this_buf.append("")
        buf.extend(this_buf)

    if "variable" in symbols:
        this_buf = [
            "Constants",
            "---------",
            "",
            ".. toctree::",
            "   :maxdepth: 1",
            "",
        ]

        for name, props in symbols["variable"]:
            child_file = f"variable-{name}.rst"
            this_buf.append(f"   {child_file}")

            header = Path(props.location).relative_to(include_dir)

            full_name = f"{ns}::{name}"
            block = [
                name,
                "=" * len(name),
                "",
                f"Defined in header: <{header}>",
                "",
                f".. doxygenvariable:: {full_name}",
            ]

            thisdir.joinpath(child_file).write_text("\n".join(block))

        this_buf.append("")
        buf.extend(this_buf)

    if "function" in symbols:
        this_buf = [
            "Functions",
            "---------",
            "",
            ".. toctree::",
            "   :maxdepth: 1",
            "",
        ]

        for name, props in symbols["function"]:
            child_file = f"function-{name}.rst"
            this_buf.append(f"   {child_file}")

            header = Path(props.location).relative_to(include_dir)

            full_name = f"{ns}::{name}{props.argsstring}"
            block = [
                f"{name}",
                "=" * len(name),
                "",
                f"Defined in header: <{header}>",
                "",
                f".. doxygenfunction:: {full_name}",
            ]

            thisdir.joinpath(child_file).write_text("\n".join(block))

        this_buf.append("")
        buf.extend(this_buf)

    if "struct" in symbols:
        this_buf = [
            "Struct",
            "------",
            "",
            ".. toctree::",
            "   :maxdepth: 1",
            "",
        ]

        for name, props in symbols["struct"]:
            child_file = f"struct-{name}.rst"
            this_buf.append(f"   {child_file}")

            header = Path(props.location).relative_to(include_dir)

            full_name = f"{ns}::{name}"
            block = [
                name,
                "=" * len(name),
                "",
                f"Defined in header: <{header}>",
                "",
                f".. doxygenstruct:: {full_name}",
                f"   :members:",
            ]

            thisdir.joinpath(child_file).write_text("\n".join(block))

        this_buf.append("")
        buf.extend(this_buf)

    if "class" in symbols:
        this_buf = [
            "Classes",
            "-------",
            "",
            ".. toctree::",
            "   :maxdepth: 1",
            "",
        ]

        for name, props in symbols["class"]:
            child_file = f"class-{name}.rst"
            this_buf.append(f"   {child_file}")

            header = Path(props.location).relative_to(include_dir)

            full_name = f"{ns}::{name}"
            block = [
                name,
                "=" * len(name),
                "",
                f"Defined in header: <{header}>",
                "",
                f".. doxygenclass:: {full_name}",
                f"   :members:",
                "",
            ]

            thisdir.joinpath(child_file).write_text("\n".join(block))

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
        index_file.write_text(generate_rst(thisdir, scope, child_scopes, symbols))

    symbol_tree.walk(_func)


def main():
    parser = argparse.ArgumentParser(
        description="Parse doxygen XML outputs and perform actions.",
        epilog="""To re-generate the C++ API Reference, run this script with
        --mode doctree and specify the output directory to ./doc/cpp.  To re-generate
        the enum-stream-test code, run this script with --mode enum-test and specify
        the output directory to ./src.
        """
    )
    parser.add_argument("--mode", type=str, required=True, help="Type of action to perform.")
    parser.add_argument("--output", "-o", type=Path, help="Output directory path.")
    parser.add_argument("rootdir", type=Path, help="Directory where the doxygen XML files are found.")
    args = parser.parse_args()

    actions = {
        "kinds": (list_kinds, (args.rootdir,)),
        "symbols": (dump_symbols, (args.rootdir,)),
        "enum-test": (create_enum_stream_test, (args.rootdir, args.output)),
        "doctree": (generate_doctree, (args.rootdir, args.output))
    }

    action = actions.get(args.mode)
    if not action:
        print(f"unknown mode: {args.mode}", file=sys.stderr)
        sys.exit(1)

    func, args = action
    func(*args)


if __name__ == "__main__":
    main()
