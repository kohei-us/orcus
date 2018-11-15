/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "orcus/spreadsheet/types.hpp"
#include "orcus/global.hpp"
#include "orcus/exception.hpp"

#include <limits>
#include <sstream>

#include <mdds/sorted_string_map.hpp>

namespace orcus { namespace spreadsheet {

namespace {

typedef mdds::sorted_string_map<totals_row_function_t> trf_map_type;
typedef mdds::sorted_string_map<pivot_cache_group_by_t> pc_group_by_map_type;
typedef mdds::sorted_string_map<error_value_t> error_value_map_type;

// Keys must be sorted.
trf_map_type::entry trf_entries[] =
{
    { ORCUS_ASCII("average"),   totals_row_function_t::average },
    { ORCUS_ASCII("count"),     totals_row_function_t::count },
    { ORCUS_ASCII("countNums"), totals_row_function_t::count_numbers },
    { ORCUS_ASCII("custom"),    totals_row_function_t::custom },
    { ORCUS_ASCII("max"),       totals_row_function_t::maximum },
    { ORCUS_ASCII("min"),       totals_row_function_t::minimum },
    { ORCUS_ASCII("none"),      totals_row_function_t::none },
    { ORCUS_ASCII("stdDev"),    totals_row_function_t::standard_deviation },
    { ORCUS_ASCII("sum"),       totals_row_function_t::sum },
    { ORCUS_ASCII("var"),       totals_row_function_t::variance },
};

const trf_map_type& get_trf_map()
{
    static trf_map_type trf_map(
        trf_entries,
        sizeof(trf_entries)/sizeof(trf_entries[0]),
        totals_row_function_t::none);

    return trf_map;
}

// Keys must be sorted.
pc_group_by_map_type::entry pc_group_by_entries[] =
{
    { ORCUS_ASCII("days"),     pivot_cache_group_by_t::days },
    { ORCUS_ASCII("hours"),    pivot_cache_group_by_t::hours },
    { ORCUS_ASCII("minutes"),  pivot_cache_group_by_t::minutes },
    { ORCUS_ASCII("months"),   pivot_cache_group_by_t::months },
    { ORCUS_ASCII("quarters"), pivot_cache_group_by_t::quarters },
    { ORCUS_ASCII("range"),    pivot_cache_group_by_t::range },
    { ORCUS_ASCII("seconds"),  pivot_cache_group_by_t::seconds },
    { ORCUS_ASCII("years"),    pivot_cache_group_by_t::years },
};

const pc_group_by_map_type& get_pc_group_by_map()
{
    static pc_group_by_map_type pc_group_by_map(
        pc_group_by_entries,
        ORCUS_N_ELEMENTS(pc_group_by_entries),
        pivot_cache_group_by_t::unknown);

    return pc_group_by_map;
}

// Keys must be sorted.
error_value_map_type::entry error_value_entries[] =
{
    { ORCUS_ASCII("#DIV/0!"), error_value_t::div0  },
    { ORCUS_ASCII("#N/A!"),   error_value_t::na    },
    { ORCUS_ASCII("#NAME?"),  error_value_t::name  },
    { ORCUS_ASCII("#NULL!"),  error_value_t::null  },
    { ORCUS_ASCII("#NUM!"),   error_value_t::num   },
    { ORCUS_ASCII("#REF!"),   error_value_t::ref   },
    { ORCUS_ASCII("#VALUE!"), error_value_t::value },
};

const error_value_map_type& get_error_value_map()
{
    static error_value_map_type error_value_map(
        error_value_entries,
        ORCUS_N_ELEMENTS(error_value_entries),
        error_value_t::unknown);

    return error_value_map;
}

}

color_rgb_t::color_rgb_t() : red(0), green(0), blue(0) {}

color_rgb_t::color_rgb_t(const color_rgb_t& other) :
    red(other.red), green(other.green), blue(other.blue) {}

color_rgb_t::color_rgb_t(color_rgb_t&& other) :
    red(other.red), green(other.green), blue(other.blue)
{
    other.red = 0;
    other.green = 0;
    other.blue = 0;
}

bool operator== (const address_t& left, const address_t& right)
{
    return left.column == right.column && left.row == right.row;
}

bool operator!= (const address_t& left, const address_t& right)
{
    return !operator== (left, right);
}

bool operator== (const range_t& left, const range_t& right)
{
    return left.first == right.first && left.last == right.last;
}

bool operator!= (const range_t& left, const range_t& right)
{
    return !operator== (left, right);
}

bool operator< (const range_t& left, const range_t& right)
{
    if (left.first.row != right.first.row)
        return left.first.row < right.first.row;

    if (left.first.column != right.first.column)
        return left.first.column < right.first.column;

    if (left.last.row != right.last.row)
        return left.last.row < right.last.row;

    return left.last.column < right.last.column;
}

bool operator> (const range_t& left, const range_t& right)
{
    if (left.first.row != right.first.row)
        return left.first.row > right.first.row;

    if (left.first.column != right.first.column)
        return left.first.column > right.first.column;

    if (left.last.row != right.last.row)
        return left.last.row > right.last.row;

    return left.last.column > right.last.column;
}

range_t& operator+= (range_t& left, const address_t& right)
{
    left.first.column += right.column;
    left.first.row    += right.row;
    left.last.column  += right.column;
    left.last.row     += right.row;

    return left;
}

range_t& operator-= (range_t& left, const address_t& right)
{
    left.first.column -= right.column;
    left.first.row    -= right.row;
    left.last.column  -= right.column;
    left.last.row     -= right.row;

    return left;
}

std::ostream& operator<< (std::ostream& os, const address_t& v)
{
    os << "(column=" << v.column << ",row=" << v.row << ")";
    return os;
}

std::ostream& operator<< (std::ostream& os, const range_t& v)
{
    os << v.first << "-" << v.last;
    return os;
}

color_rgb_t& color_rgb_t::operator= (const color_rgb_t& other)
{
    red = other.red;
    green = other.green;
    blue = other.blue;

    return *this;
}

col_width_t get_default_column_width()
{
    return std::numeric_limits<col_width_t>::max();
}

row_height_t get_default_row_height()
{
    return std::numeric_limits<row_height_t>::max();
}

totals_row_function_t to_totals_row_function_enum(const char* p, size_t n)
{
    return get_trf_map().find(p, n);
}

pivot_cache_group_by_t to_pivot_cache_group_by_enum(const char* p, size_t n)
{
    return get_pc_group_by_map().find(p, n);
}

error_value_t to_error_value_enum(const char* p, size_t n)
{
    return get_error_value_map().find(p, n);
}

color_rgb_t to_color_rgb(const char* p, size_t n)
{
    // RGB string is a 6-character string representing 24-bit hexadecimal
    // number e.g. '004A12' (red - green - blue)

    // store the original head position and size.
    const char* p0 = p;
    size_t n0 = n;

    if (n == 7 && *p == '#')
    {
        // Skip the leading '#' character.
        --n;
        ++p;
    }

    if (n != 6)
    {
        std::ostringstream os;
        os << "'" << pstring(p0, n0) << "' is not a valid RGB color string.";
        throw value_error(os.str());
    }

    color_rgb_t ret;
    long converted = 0;
    const char* p_end = p + n;

    for (; p != p_end; ++p)
    {
        converted <<= 4;

        char c = *p;
        long this_val = 0;

        if ('0' <= c && c <= '9')
            this_val = c - '0';
        else if ('a' <= c && c <= 'f')
            this_val = 10 + c - 'a';
        else if ('A' <= c && c <= 'F')
            this_val = 10 + c - 'A';
        else
        {
            std::ostringstream os;
            os << "'" << pstring(p0, n0) << "' is not a valid RGB color string.";
            throw value_error(os.str());
        }

        converted += this_val;
    }

    ret.blue  = (0x000000FF & converted);
    converted >>= 8;
    ret.green = (0x000000FF & converted);
    converted >>= 8;
    ret.red   = (0x000000FF & converted);

    return ret;
}

std::ostream& operator<< (std::ostream& os, error_value_t ev)
{
    switch (ev)
    {
        case error_value_t::div0:
            os << error_value_entries[0].key;
            break;
        case error_value_t::na:
            os << error_value_entries[1].key;
            break;
        case error_value_t::name:
            os << error_value_entries[2].key;
            break;
        case error_value_t::null:
            os << error_value_entries[3].key;
            break;
        case error_value_t::num:
            os << error_value_entries[4].key;
            break;
        case error_value_t::ref:
            os << error_value_entries[5].key;
            break;
        case error_value_t::value:
            os << error_value_entries[6].key;
            break;
        case error_value_t::unknown:
        default:
            ;
    }
    return os;
}

std::ostream& operator<< (std::ostream& os, formula_grammar_t grammar)
{
    static const std::vector<const char*> entries = {
        "unknown",
        "xls_xml",
        "xlsx",
        "ods",
        "gnumeric"
    };

    size_t n = static_cast<size_t>(grammar);
    if (n >= entries.size())
        n = 0; // unknown

    os << entries[n];
    return os;
}

std::ostream& operator<< (std::ostream& os, const color_rgb_t& color)
{
    os << "(r=" << (int)color.red << ",g=" << (int)color.green << ",b=" << (int)color.blue << ")";
    return os;
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
