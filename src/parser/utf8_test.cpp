/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "test_global.hpp"
#include "utf8.hpp"

#include <iostream>
#include <vector>
#include <string>
#include <cassert>
#include <functional>
#include <iomanip>

using namespace orcus;
using std::cout;
using std::endl;

struct cp_range_t
{
    uint32_t lower;
    uint32_t upper;
    bool valid;
};

using parse_func_t = std::function<const char*(const char*, const char*)>;

bool check_cp_ranges(parse_func_t parse, std::vector<cp_range_t> ranges)
{
    for (const cp_range_t& range : ranges)
    {
        for (uint32_t cp = range.lower; cp <= range.upper; ++cp)
        {
            std::vector<char> buf;

            try
            {
                buf = encode_utf8(cp);
            }
            catch (const std::exception& e)
            {
                cout << "failed to encode 0x" << std::hex << std::uppercase << cp
                     << " as utf-8: " << e.what() << endl;
                return false;
            }

            const char* p = buf.data();
            const char* p_end = p + buf.size();
            const char* ret = parse(p, p_end);

            if ((ret == p_end) != range.valid)
            {
                cout << "failed to parse 0x" << std::hex << std::uppercase << cp
                    << " (utf-8:";

                for (char b : buf)
                    cout << ' ' << short(0xFF & b);
                cout << ")" << endl;
                cout << "expected to be " << (range.valid ? "valid" : "invalid")
                     << ", but was " << (range.valid ? "invalid" : "valid") << endl;
                return false;
            }
        }
    }

    return true;
}

void test_xml_name_start_char()
{
    bool res = check_cp_ranges(
        parse_utf8_xml_name_start_char,
        {
            { 0x00, 0x40, false },
            { 'A', 'Z', true },
            { '[', '^', false },
            { '_', '_', true },
            { '`', '`', false },
            { 'a', 'z', true },
            { '{', 0xBF, false },
            { 0xC0, 0xD6, true },
            { 0xD7, 0xD7, false },
            { 0xD8, 0xF6, true },
            { 0xF7, 0xF7, false },
            { 0xF8, 0x2FF, true },
            { 0x300, 0x36F, false },
            { 0x370, 0x37D, true },
            { 0x37E, 0x37E, false },
            { 0x37F, 0x1FFF, true },
            { 0x2000, 0x200B, false },
            { 0x200C, 0x200D, true },
            { 0x200E, 0x206F, false },
            { 0x2070, 0x218F, true },
            { 0x2190, 0x2BFF, false },
            { 0x2C00, 0x2FEF, true },
            { 0x2FF0, 0x3000, false },
            { 0x3001, 0xD7FF, true },
            { 0xD800, 0xF8FF, false },
            { 0xF900, 0xFDCF, true },
            { 0xFDD0, 0xFDEF, false },
            { 0xFDF0, 0xFFFD, true },
            { 0xFFFE, 0xFFFF, false },
            { 0x10000, 0xEFFFF, true },
            { 0xF0000, 0xF0000, false }, // just check one byte past last valid byte.
        }
    );
    assert(res);
}

void test_xml_name_char()
{
    bool res = check_cp_ranges(
        parse_utf8_xml_name_char,
        {
            { 0x00, ',', false },
            { '-', '.', true }, // 0x2D - 0x2E
            { '/', '/', false },
            { '0', '9', true },
            { ':', '@', false },
            { 'A', 'Z', true },
            { '[', '^', false },
            { '_', '_', true }, // 0x5F
            { '`', '`', false },
            { 'a', 'z', true },
            { '{', 0xB6, false },
            { 0xB7, 0xB7, true },
            { 0xB8, 0xBF, false },
            { 0xC0, 0xD6, true },
            { 0xD7, 0xD7, false },
            { 0xD8, 0xF6, true },
            { 0xF7, 0xF7, false },
            { 0xF8, 0x2FF, true },
            { 0x300, 0x36F, true },
            { 0x370, 0x37D, true },
            { 0x37E, 0x37E, false },
            { 0x37F, 0x1FFF, true },
            { 0x2000, 0x200B, false },
            { 0x200C, 0x200D, true },
            { 0x200E, 0x203E, false },
            { 0x203F, 0x2040, true },
            { 0x2041, 0x206F, false },
            { 0x2070, 0x218F, true },
            { 0x2190, 0x2BFF, false },
            { 0x2C00, 0x2FEF, true },
            { 0x2FF0, 0x3000, false },
            { 0x3001, 0xD7FF, true },
            { 0xD800, 0xF8FF, false },
            { 0xF900, 0xFDCF, true },
            { 0xFDD0, 0xFDEF, false },
            { 0xFDF0, 0xFFFD, true },
            { 0xFFFE, 0xFFFF, false },
            { 0x10000, 0xEFFFF, true },
            { 0xF0000, 0xF0000, false }, // just check one byte past last valid byte.
        }
    );
    assert(res);
}

int main(int argc, char** argv)
{
    test_xml_name_start_char();
    test_xml_name_char();

    return EXIT_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
