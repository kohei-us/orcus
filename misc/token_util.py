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

    token_id = 1
    token_size = len(tokens)
    for i in range(token_size):
        token = normalize_name(tokens[i])
        outfile.write("const xml_token_t XML_%s = %d;\n"%(token, token_id))
        token_id += 1
    outfile.write("\n")
    outfile.close()


def gen_token_names(outfile, tokens):

    outfile.write(get_auto_gen_warning())

    outfile.write("const char* token_names[] = {\n")
    outfile.write("    \"%s\", // 0\n"%unknown_token_name)
    token_id = 1
    token_size = len(tokens)
    for i in range(token_size):
        token = tokens[i]
        s = ','
        if i == token_size-1:
            s = ' '
        outfile.write("    \"%s\"%s // %d\n"%(token, s, token_id))
        token_id += 1
    outfile.write("};\n\n")
    outfile.write("size_t token_name_count = %d;\n\n"%token_id)
    outfile.close()
