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
using c2_range_t = std::pair<uint16_t, uint16_t>;
using c3_range_t = std::pair<uint32_t, uint32_t>;

bool check_c1_ranges(parse_func_t parse, std::vector<std::string> ranges)
{
    for (const std::string& range : ranges)
    {
        for (char c = range[0]; c <= range[1]; ++c)
        {
            const char* p = &c;
            const char* p_end = p + 1;
            const char* ret = parse(p, p_end);

            if (ret != p_end)
            {
                cout << "failed to parse '" << c << "'" << endl;
                return false;
            }
        }
    }

    return true;
}

bool check_c2_ranges(parse_func_t parse, std::vector<c2_range_t> ranges)
{
    for (const c2_range_t& range : ranges)
    {
        for (uint16_t v = range.first; v <= range.second; ++v)
        {
            char buf[2];
            buf[0] = 0x00FF & (v >> 8);
            buf[1] = 0x00FF & v;

            const char* p = buf;
            const char* p_end = p + 2;
            const char* ret = parse(p, p_end);

            if (ret != p_end)
            {
                cout << "failed to parse " << std::hex << std::uppercase << std::setfill('0')
                     << std::setw(2) << short(0xFF & buf[0]) << ' '
                     << std::setw(2) << short(0xFF & buf[1]) << endl;
                return false;
            }
        }
    }

    return true;
}

bool check_c3_ranges(parse_func_t parse, std::vector<c3_range_t> ranges)
{
    for (const c3_range_t& range : ranges)
    {
        for (uint32_t v = range.first; v <= range.second; ++v)
        {
            char buf[3];
            buf[0] = 0x00FF & (v >> 16);
            buf[1] = 0x00FF & (v >>  8);
            buf[2] = 0x00FF & v;

            const char* p = buf;
            const char* p_end = p + 3;
            const char* ret = parse(p, p_end);

            if (ret != p_end)
            {
                cout << "failed to parse " << std::hex << std::uppercase << std::setfill('0')
                     << std::setw(2) << short(0xFF & buf[0]) << ' '
                     << std::setw(2) << short(0xFF & buf[1]) << ' '
                     << std::setw(2) << short(0xFF & buf[2]) << endl;
                return false;
            }
        }
    }

    return true;
}

void test_xml_name_start_char()
{
    bool res = check_c1_ranges(
        parse_utf8_xml_name_start_char,
        { "az", "AZ", "__" }
    );
    assert(res);

    res = check_c2_ranges(
        parse_utf8_xml_name_start_char,
        {
            { 0xC380, 0xC396 },
            { 0xC398, 0xC3B6 },
            { 0xC3B8, 0xCBBF },
            { 0xCDB0, 0xCDBD },
            { 0xCDBF, 0xDFBF },
        }
    );
    assert(res);

    res = check_c3_ranges(
        parse_utf8_xml_name_start_char,
        {
            { 0xE0A080, 0xE1BFBF },
            { 0xE2808C, 0xE2808D },
            { 0xE281B0, 0xE2868F },
            { 0xE2B080, 0xE2BFAF },
            { 0xE38081, 0xED9FBF },
            { 0xEFA480, 0xEFB78F },
//          { 0xEFB7B0, 0xEFBFBD }, // TODO: this fails
        }
    );

    assert(res);
}

void test_xml_name_char()
{
    bool res = check_c1_ranges(
        parse_utf8_xml_name_char,
        { "az", "AZ", "09", "__", "--", ".." }
    );
    assert(res);

    res = check_c2_ranges(
        parse_utf8_xml_name_char,
        {
            { 0xC2B7, 0xC2B7 },
            { 0xC380, 0xC396 },
            { 0xC398, 0xC3B6 },
            { 0xC3B8, 0xCBBF },
            { 0xCC80, 0xCDAF },
            { 0xCDB0, 0xCDBD },
            { 0xCDBF, 0xDFBF },
        }
    );
    assert(res);

    // TODO : implement the rest.
}

int main(int argc, char** argv)
{
    test_xml_name_start_char();
    test_xml_name_char();

    return EXIT_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
