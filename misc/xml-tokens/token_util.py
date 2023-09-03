########################################################################
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
########################################################################

import sys


unknown_token_name = "??"


def normalize_name(old):
    new = ''
    for c in old:
        if c in '.-': # '.' nad '-' are not allowed in C++ symbols.
            c = '_'
        new += c
    return new


def gen_token_list(filepath, tokens, ns_tokens):
    dic = {}
    for t in tokens:
        dic[t] = True
    for t in ns_tokens:
        dic[t] = True

    keys = dic.keys()
    keys.sort()
    file = open(filepath, 'w')
    for key in keys:
        file.write(key + "\n")
    file.close()


def get_auto_gen_warning():
    return "// This file has been auto-generated.  Do not hand-edit this."


def gen_token_constants(outfile, tokens):

    with open(outfile, "w") as f:
        print(get_auto_gen_warning(), file=f)
        print(file=f)

        for i, token in enumerate(tokens):
            token = normalize_name(token)
            print(f"const xml_token_t XML_{token} = {i+1};", file=f)


def gen_token_names(outfile, tokens):

    with open(outfile, "w") as f:
        print(get_auto_gen_warning(), file=f)
        print(file=f)

        print("const char* token_names[] = {", file=f)
        print(f"    \"{unknown_token_name}\", // 0", file=f)

        for i, token in enumerate(tokens):
            s = ','
            if i == len(tokens) - 1:
                s = ' '
            print(f"    \"{token}\"{s} // {i+1}", file=f)
        print("};", file=f)
        print(file=f)
        print(f"size_t token_name_count = {len(tokens)+1};", file=f)
