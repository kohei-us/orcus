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

namespace named_colors {

using map_type = mdds::sorted_string_map<color_rgb_t>;

const std::vector<map_type::entry> entries =
{
    { ORCUS_ASCII("aliceblue"), { 0xF0, 0xF8, 0xFF } },
    { ORCUS_ASCII("antiquewhite"), { 0xFA, 0xEB, 0xD7 } },
    { ORCUS_ASCII("aquamarine"), { 0x7F, 0xFF, 0xD4 } },
    { ORCUS_ASCII("azure"), { 0xF0, 0xFF, 0xFF } },
    { ORCUS_ASCII("beige"), { 0xF5, 0xF5, 0xDC } },
    { ORCUS_ASCII("bisque"), { 0xFF, 0xE4, 0xC4 } },
    { ORCUS_ASCII("black"), { 0x00, 0x00, 0x00 } },
    { ORCUS_ASCII("blanchedalmond"), { 0xFF, 0xEB, 0xCD } },
    { ORCUS_ASCII("blue"), { 0x00, 0x00, 0xFF } },
    { ORCUS_ASCII("blueviolet"), { 0x8A, 0x2B, 0xE2 } },
    { ORCUS_ASCII("brown"), { 0xA5, 0x2A, 0x2A } },
    { ORCUS_ASCII("burlywood"), { 0xDE, 0xB8, 0x87 } },
    { ORCUS_ASCII("cadetblue"), { 0x5F, 0x9E, 0xA0 } },
    { ORCUS_ASCII("chartreuse"), { 0x7F, 0xFF, 0x00 } },
    { ORCUS_ASCII("chocolate"), { 0xD2, 0x69, 0x1E } },
    { ORCUS_ASCII("coral"), { 0xFF, 0x7F, 0x50 } },
    { ORCUS_ASCII("cornflowerblue"), { 0x64, 0x95, 0xED } },
    { ORCUS_ASCII("cornsilk"), { 0xFF, 0xF8, 0xDC } },
    { ORCUS_ASCII("crimson"), { 0xDC, 0x14, 0x3C } },
    { ORCUS_ASCII("cyan"), { 0x00, 0xFF, 0xFF } },
    { ORCUS_ASCII("darkblue"), { 0x00, 0x00, 0x8B } },
    { ORCUS_ASCII("darkcyan"), { 0x00, 0x8B, 0x8B } },
    { ORCUS_ASCII("darkgoldenrod"), { 0xB8, 0x86, 0x0B } },
    { ORCUS_ASCII("darkgray"), { 0xA9, 0xA9, 0xA9 } },
    { ORCUS_ASCII("darkgreen"), { 0x00, 0x64, 0x00 } },
    { ORCUS_ASCII("darkkhaki"), { 0xBD, 0xB7, 0x6B } },
    { ORCUS_ASCII("darkmagenta"), { 0x8B, 0x00, 0x8B } },
    { ORCUS_ASCII("darkolivegreen"), { 0x55, 0x6B, 0x2F } },
    { ORCUS_ASCII("darkorange"), { 0xFF, 0x8C, 0x00 } },
    { ORCUS_ASCII("darkorchid"), { 0x99, 0x32, 0xCC } },
    { ORCUS_ASCII("darkred"), { 0x8B, 0x00, 0x00 } },
    { ORCUS_ASCII("darksalmon"), { 0xE9, 0x96, 0x7A } },
    { ORCUS_ASCII("darkseagreen"), { 0x8F, 0xBC, 0x8F } },
    { ORCUS_ASCII("darkslateblue"), { 0x48, 0x3D, 0x8B } },
    { ORCUS_ASCII("darkslategray"), { 0x2F, 0x4F, 0x4F } },
    { ORCUS_ASCII("darkturquoise"), { 0x00, 0xCE, 0xD1 } },
    { ORCUS_ASCII("darkviolet"), { 0x94, 0x00, 0xD3 } },
    { ORCUS_ASCII("deeppink"), { 0xFF, 0x14, 0x93 } },
    { ORCUS_ASCII("deepskyblue"), { 0x00, 0xBF, 0xFF } },
    { ORCUS_ASCII("dimgray"), { 0x69, 0x69, 0x69 } },
    { ORCUS_ASCII("dodgerblue"), { 0x1E, 0x90, 0xFF } },
    { ORCUS_ASCII("firebrick"), { 0xB2, 0x22, 0x22 } },
    { ORCUS_ASCII("floralwhite"), { 0xFF, 0xFA, 0xF0 } },
    { ORCUS_ASCII("forestgreen"), { 0x22, 0x8B, 0x22 } },
    { ORCUS_ASCII("gainsboro"), { 0xDC, 0xDC, 0xDC } },
    { ORCUS_ASCII("ghostwhite"), { 0xF8, 0xF8, 0xFF } },
    { ORCUS_ASCII("gold"), { 0xFF, 0xD7, 0x00 } },
    { ORCUS_ASCII("goldenrod"), { 0xDA, 0xA5, 0x20 } },
    { ORCUS_ASCII("gray"), { 0x80, 0x80, 0x80 } },
    { ORCUS_ASCII("green"), { 0x00, 0x80, 0x00 } },
    { ORCUS_ASCII("greenyellow"), { 0xAD, 0xFF, 0x2F } },
    { ORCUS_ASCII("honeydew"), { 0xF0, 0xFF, 0xF0 } },
    { ORCUS_ASCII("hotpink"), { 0xFF, 0x69, 0xB4 } },
    { ORCUS_ASCII("indianred"), { 0xCD, 0x5C, 0x5C } },
    { ORCUS_ASCII("indigo"), { 0x4B, 0x00, 0x82 } },
    { ORCUS_ASCII("ivory"), { 0xFF, 0xFF, 0xF0 } },
    { ORCUS_ASCII("khaki"), { 0xF0, 0xE6, 0x8C } },
    { ORCUS_ASCII("lavender"), { 0xE6, 0xE6, 0xFA } },
    { ORCUS_ASCII("lavenderblush"), { 0xFF, 0xF0, 0xF5 } },
    { ORCUS_ASCII("lawngreen"), { 0x7C, 0xFC, 0x00 } },
    { ORCUS_ASCII("lemonchiffon"), { 0xFF, 0xFA, 0xCD } },
    { ORCUS_ASCII("lightblue"), { 0xAD, 0xD8, 0xE6 } },
    { ORCUS_ASCII("lightcoral"), { 0xF0, 0x80, 0x80 } },
    { ORCUS_ASCII("lightcyan"), { 0xE0, 0xFF, 0xFF } },
    { ORCUS_ASCII("lightgoldenrodyellow"), { 0xFA, 0xFA, 0xD2 } },
    { ORCUS_ASCII("lightgray"), { 0xD3, 0xD3, 0xD3 } },
    { ORCUS_ASCII("lightgreen"), { 0x90, 0xEE, 0x90 } },
    { ORCUS_ASCII("lightpink"), { 0xFF, 0xB6, 0xC1 } },
    { ORCUS_ASCII("lightsalmon"), { 0xFF, 0xA0, 0x7A } },
    { ORCUS_ASCII("lightseagreen"), { 0x20, 0xB2, 0xAA } },
    { ORCUS_ASCII("lightskyblue"), { 0x87, 0xCE, 0xFA } },
    { ORCUS_ASCII("lightslategray"), { 0x77, 0x88, 0x99 } },
    { ORCUS_ASCII("lightsteelblue"), { 0xB0, 0xC4, 0xDE } },
    { ORCUS_ASCII("lightyellow"), { 0xFF, 0xFF, 0xE0 } },
    { ORCUS_ASCII("lime"), { 0x00, 0xFF, 0x00 } },
    { ORCUS_ASCII("limegreen"), { 0x32, 0xCD, 0x32 } },
    { ORCUS_ASCII("linen"), { 0xFA, 0xF0, 0xE6 } },
    { ORCUS_ASCII("magenta"), { 0xFF, 0x00, 0xFF } },
    { ORCUS_ASCII("maroon"), { 0x80, 0x00, 0x00 } },
    { ORCUS_ASCII("mediumaquamarine"), { 0x66, 0xCD, 0xAA } },
    { ORCUS_ASCII("mediumblue"), { 0x00, 0x00, 0xCD } },
    { ORCUS_ASCII("mediumorchid"), { 0xBA, 0x55, 0xD3 } },
    { ORCUS_ASCII("mediumpurple"), { 0x93, 0x70, 0xDB } },
    { ORCUS_ASCII("mediumseagreen"), { 0x3C, 0xB3, 0x71 } },
    { ORCUS_ASCII("mediumslateblue"), { 0x7B, 0x68, 0xEE } },
    { ORCUS_ASCII("mediumspringgreen"), { 0x00, 0xFA, 0x9A } },
    { ORCUS_ASCII("mediumturquoise"), { 0x48, 0xD1, 0xCC } },
    { ORCUS_ASCII("mediumvioletred"), { 0xC7, 0x15, 0x85 } },
    { ORCUS_ASCII("midnightblue"), { 0x19, 0x19, 0x70 } },
    { ORCUS_ASCII("mintcream"), { 0xF5, 0xFF, 0xFA } },
    { ORCUS_ASCII("mistyrose"), { 0xFF, 0xE4, 0xE1 } },
    { ORCUS_ASCII("moccasin"), { 0xFF, 0xE4, 0xB5 } },
    { ORCUS_ASCII("navajowhite"), { 0xFF, 0xDE, 0xAD } },
    { ORCUS_ASCII("navy"), { 0x00, 0x00, 0x80 } },
    { ORCUS_ASCII("oldlace"), { 0xFD, 0xF5, 0xE6 } },
    { ORCUS_ASCII("olive"), { 0x80, 0x80, 0x00 } },
    { ORCUS_ASCII("olivedrab"), { 0x6B, 0x8E, 0x23 } },
    { ORCUS_ASCII("orange"), { 0xFF, 0xA5, 0x00 } },
    { ORCUS_ASCII("orangered"), { 0xFF, 0x45, 0x00 } },
    { ORCUS_ASCII("orchid"), { 0xDA, 0x70, 0xD6 } },
    { ORCUS_ASCII("palegoldenrod"), { 0xEE, 0xE8, 0xAA } },
    { ORCUS_ASCII("palegreen"), { 0x98, 0xFB, 0x98 } },
    { ORCUS_ASCII("paleturquoise"), { 0xAF, 0xEE, 0xEE } },
    { ORCUS_ASCII("palevioletred"), { 0xDB, 0x70, 0x93 } },
    { ORCUS_ASCII("papayawhip"), { 0xFF, 0xEF, 0xD5 } },
    { ORCUS_ASCII("peachpuff"), { 0xFF, 0xDA, 0xB9 } },
    { ORCUS_ASCII("peru"), { 0xCD, 0x85, 0x3F } },
    { ORCUS_ASCII("pink"), { 0xFF, 0xC0, 0xCB } },
    { ORCUS_ASCII("plum"), { 0xDD, 0xA0, 0xDD } },
    { ORCUS_ASCII("powderblue"), { 0xB0, 0xE0, 0xE6 } },
    { ORCUS_ASCII("purple"), { 0x80, 0x00, 0x80 } },
    { ORCUS_ASCII("red"), { 0xFF, 0x00, 0x00 } },
    { ORCUS_ASCII("rosybrown"), { 0xBC, 0x8F, 0x8F } },
    { ORCUS_ASCII("royalblue"), { 0x41, 0x69, 0xE1 } },
    { ORCUS_ASCII("saddlebrown"), { 0x8B, 0x45, 0x13 } },
    { ORCUS_ASCII("salmon"), { 0xFA, 0x80, 0x72 } },
    { ORCUS_ASCII("sandybrown"), { 0xF4, 0xA4, 0x60 } },
    { ORCUS_ASCII("seagreen"), { 0x2E, 0x8B, 0x57 } },
    { ORCUS_ASCII("seashell"), { 0xFF, 0xF5, 0xEE } },
    { ORCUS_ASCII("sienna"), { 0xA0, 0x52, 0x2D } },
    { ORCUS_ASCII("silver"), { 0xC0, 0xC0, 0xC0 } },
    { ORCUS_ASCII("skyblue"), { 0x87, 0xCE, 0xEB } },
    { ORCUS_ASCII("slateblue"), { 0x6A, 0x5A, 0xCD } },
    { ORCUS_ASCII("slategray"), { 0x70, 0x80, 0x90 } },
    { ORCUS_ASCII("snow"), { 0xFF, 0xFA, 0xFA } },
    { ORCUS_ASCII("springgreen"), { 0x00, 0xFF, 0x7F } },
    { ORCUS_ASCII("steelblue"), { 0x46, 0x82, 0xB4 } },
    { ORCUS_ASCII("tan"), { 0xD2, 0xB4, 0x8C } },
    { ORCUS_ASCII("teal"), { 0x00, 0x80, 0x80 } },
    { ORCUS_ASCII("thistle"), { 0xD8, 0xBF, 0xD8 } },
    { ORCUS_ASCII("tomato"), { 0xFF, 0x63, 0x47 } },
    { ORCUS_ASCII("turquoise"), { 0x40, 0xE0, 0xD0 } },
    { ORCUS_ASCII("violet"), { 0xEE, 0x82, 0xEE } },
    { ORCUS_ASCII("wheat"), { 0xF5, 0xDE, 0xB3 } },
    { ORCUS_ASCII("white"), { 0xFF, 0xFF, 0xFF } },
    { ORCUS_ASCII("whitesmoke"), { 0xF5, 0xF5, 0xF5 } },
    { ORCUS_ASCII("yellow"), { 0xFF, 0xFF, 0x00 } },
    { ORCUS_ASCII("yellowgreen"), { 0x9A, 0xCD, 0x32 } },
};

const map_type& get()
{
    static map_type mt(entries.data(), entries.size(), { 0x00, 0x00, 0x00 });
    return mt;
}

} // namespace named_color

} // anonymous namespace

color_rgb_t::color_rgb_t() : red(0), green(0), blue(0) {}

color_rgb_t::color_rgb_t(std::initializer_list<color_elem_t> vs)
{
    if (vs.size() != 3u)
    {
        std::ostringstream os;
        os << "color_rgb_t requires exactly 3 input values. " << vs.size() << " was given.";
        throw std::invalid_argument(os.str());
    }

    auto it = vs.begin();

    red = *it++;
    green = *it++;
    blue = *it;
}

color_rgb_t::color_rgb_t(const color_rgb_t& other) :
    red(other.red), green(other.green), blue(other.blue) {}

color_rgb_t::color_rgb_t(color_rgb_t&& other) :
    red(other.red), green(other.green), blue(other.blue)
{
    other.red = 0;
    other.green = 0;
    other.blue = 0;
}

address_t to_rc_address(const src_address_t& r)
{
    address_t ret;
    ret.row = r.row;
    ret.column = r.column;
    return ret;
}

range_t to_rc_range(const src_range_t& r)
{
    range_t ret;
    ret.first.row = r.first.row;
    ret.first.column = r.first.column;
    ret.last.row = r.last.row;
    ret.last.column = r.last.column;
    return ret;
}

bool operator== (const address_t& left, const address_t& right)
{
    return left.column == right.column && left.row == right.row;
}

bool operator!= (const address_t& left, const address_t& right)
{
    return !operator== (left, right);
}

bool operator== (const src_address_t& left, const src_address_t& right)
{
    return left.column == right.column && left.row == right.row;
}

bool operator!= (const src_address_t& left, const src_address_t& right)
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

bool operator== (const src_range_t& left, const src_range_t& right)
{
    return left.first == right.first && left.last == right.last;
}

bool operator!= (const src_range_t& left, const src_range_t& right)
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

std::ostream& operator<< (std::ostream& os, const src_address_t& v)
{
    os << "(sheet=" << v.sheet << ",column=" << v.column << ",row=" << v.row << ")";
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

color_rgb_t to_color_rgb_from_name(const char* p, size_t n)
{
    return named_colors::get().find(p, n);
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
