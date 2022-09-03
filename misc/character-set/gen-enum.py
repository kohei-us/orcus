#!/usr/bin/env python3
########################################################################
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
########################################################################

import argparse
import csv
import io
from pathlib import Path


def _cleanse_symbol(s):
    s = s.replace("-", "_")
    s = s.replace(":", "_")
    s = s.replace(".", "_")
    s = s.replace("(", "")
    s = s.replace(")", "")
    return s.lower()


def _generate_enum(enum_symbols, outpath):
    enum_symbols = sorted(enum_symbols)
    buf = list()
    buf.append("enum class character_set_t")
    buf.append("{")
    buf.append("    unspecified = 0,")

    for entry in enum_symbols:
        buf.append(f"    {entry[0]},")

    buf.append("};")

    outpath.write_text("\n".join(buf))


def _generate_map_entries(aliases, outpath):
    entries = list()
    for symbol, mapped_strs in aliases.items():
        for mapped_str in mapped_strs:
            entries.append((mapped_str.lower(), symbol))

    entries = sorted(entries, key=lambda x: x[0])
    buf = ["constexpr map_type::entry entries[] = {",]

    for entry in entries:
        buf.append(f'    {{ "{entry[0]}", character_set_t::{entry[1]} }},')

    buf.append("};")

    outpath.write_text("\n".join(buf))


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--enum-out", type=Path, default=Path("./enum.inl"))
    parser.add_argument("--map-out", type=Path, default=Path("./map-entries.inl"))
    parser.add_argument("filepath", type=Path)
    args = parser.parse_args()

    content = args.filepath.read_text()
    stream = io.StringIO(content)

    reader = csv.reader(stream)
    next(reader)  # skip the header row
    aliases = dict()
    enum_symbols = list()
    symbol = None
    for row in reader:
        mime_name, name, alias = row

        if mime_name:
            # Take the MIME name as new symbol.
            symbol = _cleanse_symbol(mime_name)
            aliases[symbol] = set([mime_name, name])
            enum_symbols.append((symbol, mime_name, name))
            if alias:
                aliases[symbol].add(alias)
        elif name:
            # Take the name as new symbol.
            symbol = _cleanse_symbol(name)
            aliases[symbol] = set([name,])
            enum_symbols.append((symbol, name))
            if alias:
                aliases[symbol].add(alias)
        else:
            # the row only contains an alias for the current symbol.
            if not alias:
                raise RuntimeError("alias must be present.")
            aliases[symbol].add(alias)

    _generate_enum(enum_symbols, args.enum_out)
    _generate_map_entries(aliases, args.map_out)


if __name__ == "__main__":
    main()

