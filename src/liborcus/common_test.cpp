/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <orcus/global.hpp>
#include <orcus/measurement.hpp>
#include <orcus/spreadsheet/types.hpp>
#include "pstring.hpp"

#include <cstdlib>
#include <cassert>
#include <iostream>
#include <sstream>
#include <cmath>

using namespace std;
using namespace orcus;

void test_date_time_conversion()
{
    struct {
        const char* str;
        int year;
        int month;
        int day;
        int hour;
        int minute;
        double second;
    } tests[] = {
        { "2011-02-12", 2011, 2, 12, 0, 0, 0.0 },
        { "1934-12-23T21:34:56.69", 1934, 12, 23, 21, 34, 56.69 },
    };

    for (size_t i = 0, n = sizeof(tests)/sizeof(tests[0]); i < n; ++i)
    {
        pstring str(tests[i].str);
        date_time_t ret = to_date_time(str);
        cout << "original: " << str << endl;
        cout << "converted: year=" << ret.year << ", month=" << ret.month << ", day="
            << ret.day << ", hour=" << ret.hour << ", minute=" << ret.minute << ", second=" << ret.second << endl;
        assert(ret.year == tests[i].year);
        assert(ret.month == tests[i].month);
        assert(ret.day == tests[i].day);
        assert(ret.hour == tests[i].hour);
        assert(ret.minute == tests[i].minute);
        assert(ret.second == tests[i].second);
    }
}

string to_string(length_unit_t unit)
{
    switch (unit)
    {
        case length_unit_t::centimeter:
            return "centimeter";
        case length_unit_t::inch:
            return "inch";
        case length_unit_t::point:
            return "point";
        case length_unit_t::twip:
            return "twip";
        case length_unit_t::unknown:
        default:
            ;
    }
    return "unknown";
}

void test_measurement_conversion()
{
    struct {
        const char* str;
        double expected;
        size_t decimals;
        length_unit_t unit;
    } tests[] = {
        { "12.34", 12.34, 2, length_unit_t::unknown },
        { "35", 35, 0, length_unit_t::unknown },
        { "0.69825", 0.69825, 5, length_unit_t::unknown },
        { ".1592", 0.1592, 4, length_unit_t::unknown },
        { "5", 5.0, 0, length_unit_t::unknown },
        { "-3", -3.0, 0, length_unit_t::unknown },
        { "-3.456", -3.456, 3, length_unit_t::unknown },
        { "-.987", -0.987, 3, length_unit_t::unknown },
        { "-100.987.", -100.987, 3, length_unit_t::unknown }, // Second decimal point should stop the parsing.

        { "12.345in", 12.345, 3, length_unit_t::inch },
        { "120.30001cm", 120.30001, 5, length_unit_t::centimeter },
        { "3.12mm", 3.12, 2, length_unit_t::millimeter },
        { "32.681pt", 32.681, 2, length_unit_t::point },
        { "0.1234px", 0.1234, 4, length_unit_t::pixel },
    };

    for (size_t i = 0, n = sizeof(tests)/sizeof(tests[0]); i < n; ++i)
    {
        length_t ret = to_length(tests[i].str);
        cout << "original: '" << tests[i].str << "', converted: " << ret.value
            << " (" << to_string(ret.unit) << "), expected: "
            << tests[i].expected << " (" << to_string(tests[i].unit) << ")" << endl;

        // Check for double-precision equality without the rounding error.
        double factor = 1.0;
        for (size_t j = 0; j < tests[i].decimals; ++j)
            factor *= 10.0;

        double converted = round(ret.value * factor);
        double expected = round(tests[i].expected * factor);
        assert(converted == expected);

        assert(ret.unit == tests[i].unit);
    }
}

void test_measurement_conversion_2()
{
    struct check
    {
        length_unit_t from;
        length_unit_t to;
        double original;
        double expected;
    };

    std::vector<check> checks =
    {
        { length_unit_t::millimeter, length_unit_t::twip, 254.0, 14400.0 },
    };

    for (const check& c : checks)
    {
        // without volatile, sometimes the following assert evaluates to false on 32-bit debian platforms.
        volatile double observed = convert(c.original, c.from, c.to);
        assert(observed == c.expected);
    }
}

void test_string2number_conversion()
{
    struct {
        const char* str;
        double expected;
        size_t decimals;
        size_t end_pos;
    } tests[] = {
        { "1.2", 1.2, 1, 3 },
        { "1.2a", 1.2, 1, 3 },
        { "1.2.", 1.2, 1, 3 },
        { "1.3456", 1.3456, 4, 6 },
        { "-10.345", -10.345, 3, 7 },
        { "-10.345-", -10.345, 3, 7 },
        { "1.2E-2", 1.2E-2, 3, 6 },
        { "1.26E+3", 1.26E+3, 1, 7 },
    };

    for (size_t i = 0, n = sizeof(tests)/sizeof(tests[0]); i < n; ++i)
    {
        const char* p = tests[i].str;
        const char* p_end = p + strlen(p);
        const char* p_parse_ended = nullptr;
        double converted = to_double(p, p_end, &p_parse_ended);
        cout << "original: '" << tests[i].str << "', converted: " << converted
            << ", expected: " << tests[i].expected << endl;

        // Check for double-precision equality without the rounding error.
        double factor = 1.0;
        for (size_t j = 0; j < tests[i].decimals; ++j)
            factor *= 10.0;

        converted = round(converted * factor);
        double expected = round(tests[i].expected * factor);
        assert(converted == expected);

        // Check the end parse position.
        const char* pos_expected = p + tests[i].end_pos;
        assert(pos_expected == p_parse_ended);
    }
}

void test_string2long_conversion()
{
    struct {
        const char* str;
        long expected;
        size_t end_pos;
    } tests[] = {
        { "1", 1, 1 },
        { "12", 12, 2 },
        { "13.4", 13, 2 },
        { "-23", -23, 3 },
        { "678abc", 678, 3 },
    };

    for (size_t i = 0, n = sizeof(tests)/sizeof(tests[0]); i < n; ++i)
    {
        const char* p = tests[i].str;
        const char* p_end = p + strlen(p);
        const char* p_parse_ended = nullptr;
        long converted = to_long(p, p_end, &p_parse_ended);
        cout << "original: '" << tests[i].str << "', converted: " << converted
            << ", expected: " << tests[i].expected << endl;

        assert(converted == tests[i].expected);

        // Check the end parse position.
        const char* pos_expected = p + tests[i].end_pos;
        assert(pos_expected == p_parse_ended);
    }
}

void test_pstring_trim()
{
    // test for trimming.
    string s1("test"), s2("  test"), s3("   test  "), s4("test   ");
    pstring ps1(s1.c_str()), ps2(s2.c_str()), ps3(s3.c_str()), ps4(s4.c_str());
    assert(ps1 != ps2);
    assert(ps1 != ps3);
    assert(ps2 != ps3);
    assert(ps1 != ps4);

    pstring trimmed = ps1.trim();
    assert(ps1 == trimmed); // nothing to trim.
    assert(ps1 == ps2.trim());
    assert(ps1 == ps3.trim());
    assert(ps1 == ps4.trim());
    assert(ps1.size() == ps2.trim().size());
    assert(ps1.size() == ps3.trim().size());
}

void test_pstring_equality()
{
    // Two pstrings that point to the same buffer but different sizes should
    // not be equal.
    string s1("some string");
    pstring p1(s1.data(), 3), p2(s1.data(), 6);
    assert(p1 != p2);

    // Two pstrings that point to different buffers but with same content
    // should be equal.
    string s2("some string");
    pstring p3(s2.data(), 6);
    assert(p2 == p3);
}

void test_spreadsheet_types()
{
    std::vector<spreadsheet::error_value_t> values =
    {
        spreadsheet::error_value_t::div0,
        spreadsheet::error_value_t::na,
        spreadsheet::error_value_t::name,
        spreadsheet::error_value_t::null,
        spreadsheet::error_value_t::num,
        spreadsheet::error_value_t::ref,
        spreadsheet::error_value_t::value
    };

    for (spreadsheet::error_value_t ev : values)
    {
        // Round-trip each enum value.
        std::ostringstream os;
        os << ev;
        std::string s = os.str();
        auto converted = spreadsheet::to_error_value_enum(s.data(), s.size());
        assert(converted == ev);
    }
}

int main()
{
    test_date_time_conversion();
    test_measurement_conversion();
    test_measurement_conversion_2();
    test_string2number_conversion();
    test_string2long_conversion();
    test_pstring_trim();
    test_pstring_equality();
    test_spreadsheet_types();

    return EXIT_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
