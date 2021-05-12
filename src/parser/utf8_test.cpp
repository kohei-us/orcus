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

using parse_func_t = std::function<const char*(const char*, const char*)>;
using cp_range_t = std::pair<uint32_t, uint32_t>;

bool check_cp_ranges(parse_func_t parse, std::vector<cp_range_t> ranges)
{
    for (const cp_range_t& range : ranges)
    {
        for (uint32_t cp = range.first; cp <= range.second; ++cp)
        {
            std::vector<char> buf = encode_utf8(cp);

            const char* p = buf.data();
            const char* p_end = p + buf.size();
            const char* ret = parse(p, p_end);

            if (ret != p_end)
            {
                cout << "failed to parse 0x" << std::hex << std::uppercase << cp
                    << " (utf-8:";

                for (char b : buf)
                    cout << ' ' << short(0xFF & b);
                cout << ")" << endl;
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
            { 'a', 'z' },
            { 'A', 'Z' },
            { '_', '_' },
            { 0xC0, 0xD6 },
            { 0xD8, 0xF6 },
            { 0xF8, 0x2FF },
            { 0x370, 0x37D },
            { 0x37F, 0x1FFF },
            { 0x200C, 0x200D },
            { 0x2070, 0x218F },
            { 0x2C00, 0x2FEF },
            { 0x3001, 0xD7FF },
            { 0xF900, 0xFDCF },
            { 0xFDF0, 0xFFFD },
            { 0x10000, 0xEFFFF },
        }
    );
    assert(res);
}

void test_xml_name_char()
{
    bool res = check_cp_ranges(
        parse_utf8_xml_name_char,
        {
            { 'a', 'z' },
            { 'A', 'Z' },
            { '0', '9' },
            { '_', '_' },
            { '-', '-' },
            { '.', '.' },
            { 0xB7, 0xB7 },
            { 0xC0, 0xD6 },
            { 0xD8, 0xF6 },
            { 0xF8, 0x2FF },
            { 0x300, 0x36F },
            { 0x370, 0x37D },
            { 0x37F, 0x1FFF },
            { 0x200C, 0x200D },
            { 0x203F, 0x2040 },
            { 0x2070, 0x218F },
            { 0x2C00, 0x2FEF },
            { 0x3001, 0xD7FF },
            { 0xF900, 0xFDCF },
            { 0xFDF0, 0xFFFD },
            { 0x10000, 0xEFFFF },
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
