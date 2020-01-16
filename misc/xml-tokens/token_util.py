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
    return "// This file has been auto-generated.  Do not hand-edit this.\n\n"


def gen_token_constants(outfile, tokens):

    outfile.write(get_auto_gen_warning())

    for i, token in enumerate(tokens):
        token = normalize_name(token)
        outfile.write(f"const xml_token_t XML_{token} = {i+1};\n")


def gen_token_names(outfile, tokens):

    outfile.write(get_auto_gen_warning())
    outfile.write("const char* token_names[] = {\n")
    outfile.write(f"    \"{unknown_token_name}\", // 0\n")

    for i, token in enumerate(tokens):
        s = ','
        if i == len(tokens) - 1:
            s = ' '
        outfile.write(f"    \"{token}\"{s} // {i+1}\n")
    outfile.write("};\n\n")
    outfile.write(f"size_t token_name_count = {len(tokens)+1};")
