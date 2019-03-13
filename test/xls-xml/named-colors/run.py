#!/usr/bin/env python3

import xml.etree.ElementTree as ET
import sys
import re


def build_color_map(infile):
    tree = ET.parse(infile)
    root = tree.getroot()
    ns = {"n": "urn:schemas-microsoft-com:office:spreadsheet"}
    nodes = root.findall("./n:Worksheet/n:Table/n:Row", ns)
    colors = {}
    for node in nodes[1:]:
        vs = node.findall("./n:Cell/n:Data", ns)
        assert(len(vs) == 2)
        v, k = vs[0].text, vs[1].text
        colors[k] = v

    return colors


def replace_color_rgb(line, colors):
    regex = re.compile(r"#([0-9]|[A-F])*", re.IGNORECASE)
    match = regex.search(line)
    rgb = line[match.span()[0]:match.span()[1]]
    name = colors.get(rgb)
    if name is None:
        raise RuntimeError("oops")
    line = regex.sub(name, line)
    return line


def main():
    colors = build_color_map(sys.argv[1])
    for k, v in colors.items():
        r = k[1:3]
        g = k[3:5]
        b = k[5:7]
        print(f"{{ ORCUS_ASCII(\"{v}\"), {{ 0x{r}, 0x{g}, 0x{b} }} }},")

    out_buffer = []
    with open(sys.argv[1], 'r') as f:
        for line in f.readlines():
            if line.find("<Interior ss:Color=") >= 0:
                line = replace_color_rgb(line, colors)
            out_buffer.append(line)

    out = "".join(out_buffer)
    with open(sys.argv[2], "w") as f:
        f.write(out)


if __name__ == "__main__":
    main()
