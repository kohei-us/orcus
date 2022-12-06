/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <orcus/spreadsheet/types.hpp>
#include <orcus/exception.hpp>
#include "pstring.hpp"

#include <limits>
#include <sstream>

#include <mdds/sorted_string_map.hpp>

namespace orcus { namespace spreadsheet {

namespace {

namespace trf {

using map_type = mdds::sorted_string_map<totals_row_function_t, mdds::string_view_map_entry>;

// Keys must be sorted.
constexpr map_type::entry entries[] = {
    { "average",   totals_row_function_t::average },
    { "count",     totals_row_function_t::count },
    { "countNums", totals_row_function_t::count_numbers },
    { "custom",    totals_row_function_t::custom },
    { "max",       totals_row_function_t::maximum },
    { "min",       totals_row_function_t::minimum },
    { "none",      totals_row_function_t::none },
    { "stdDev",    totals_row_function_t::standard_deviation },
    { "sum",       totals_row_function_t::sum },
    { "var",       totals_row_function_t::variance },
};

const map_type& get()
{
    static const map_type map(entries, std::size(entries), totals_row_function_t::none);
    return map;
}

} // namespace trf

namespace pc_group_by {

using map_type = mdds::sorted_string_map<pivot_cache_group_by_t, mdds::string_view_map_entry>;

// Keys must be sorted.
constexpr map_type::entry entries[] = {
    { "days",     pivot_cache_group_by_t::days },
    { "hours",    pivot_cache_group_by_t::hours },
    { "minutes",  pivot_cache_group_by_t::minutes },
    { "months",   pivot_cache_group_by_t::months },
    { "quarters", pivot_cache_group_by_t::quarters },
    { "range",    pivot_cache_group_by_t::range },
    { "seconds",  pivot_cache_group_by_t::seconds },
    { "years",    pivot_cache_group_by_t::years },
};

const map_type& get()
{
    static const map_type map(entries, std::size(entries), pivot_cache_group_by_t::unknown);
    return map;
}

} // namespace pc_group_by

namespace error_value {

using map_type = mdds::sorted_string_map<error_value_t, mdds::string_view_map_entry>;

// Keys must be sorted.
constexpr map_type::entry entries[] =
{
    { "#DIV/0!", error_value_t::div0  },
    { "#N/A!",   error_value_t::na    },
    { "#NAME?",  error_value_t::name  },
    { "#NULL!",  error_value_t::null  },
    { "#NUM!",   error_value_t::num   },
    { "#REF!",   error_value_t::ref   },
    { "#VALUE!", error_value_t::value },
};

const map_type& get()
{
    static const map_type map(entries, std::size(entries), error_value_t::unknown);
    return map;
}

} // namespace error_value

namespace named_colors {

using map_type = mdds::sorted_string_map<color_rgb_t, mdds::string_view_map_entry>;

constexpr map_type::entry entries[] =
{
    { "aliceblue", { 0xF0, 0xF8, 0xFF } },
    { "antiquewhite", { 0xFA, 0xEB, 0xD7 } },
    { "aquamarine", { 0x7F, 0xFF, 0xD4 } },
    { "azure", { 0xF0, 0xFF, 0xFF } },
    { "beige", { 0xF5, 0xF5, 0xDC } },
    { "bisque", { 0xFF, 0xE4, 0xC4 } },
    { "black", { 0x00, 0x00, 0x00 } },
    { "blanchedalmond", { 0xFF, 0xEB, 0xCD } },
    { "blue", { 0x00, 0x00, 0xFF } },
    { "blueviolet", { 0x8A, 0x2B, 0xE2 } },
    { "brown", { 0xA5, 0x2A, 0x2A } },
    { "burlywood", { 0xDE, 0xB8, 0x87 } },
    { "cadetblue", { 0x5F, 0x9E, 0xA0 } },
    { "chartreuse", { 0x7F, 0xFF, 0x00 } },
    { "chocolate", { 0xD2, 0x69, 0x1E } },
    { "coral", { 0xFF, 0x7F, 0x50 } },
    { "cornflowerblue", { 0x64, 0x95, 0xED } },
    { "cornsilk", { 0xFF, 0xF8, 0xDC } },
    { "crimson", { 0xDC, 0x14, 0x3C } },
    { "cyan", { 0x00, 0xFF, 0xFF } },
    { "darkblue", { 0x00, 0x00, 0x8B } },
    { "darkcyan", { 0x00, 0x8B, 0x8B } },
    { "darkgoldenrod", { 0xB8, 0x86, 0x0B } },
    { "darkgray", { 0xA9, 0xA9, 0xA9 } },
    { "darkgreen", { 0x00, 0x64, 0x00 } },
    { "darkkhaki", { 0xBD, 0xB7, 0x6B } },
    { "darkmagenta", { 0x8B, 0x00, 0x8B } },
    { "darkolivegreen", { 0x55, 0x6B, 0x2F } },
    { "darkorange", { 0xFF, 0x8C, 0x00 } },
    { "darkorchid", { 0x99, 0x32, 0xCC } },
    { "darkred", { 0x8B, 0x00, 0x00 } },
    { "darksalmon", { 0xE9, 0x96, 0x7A } },
    { "darkseagreen", { 0x8F, 0xBC, 0x8F } },
    { "darkslateblue", { 0x48, 0x3D, 0x8B } },
    { "darkslategray", { 0x2F, 0x4F, 0x4F } },
    { "darkturquoise", { 0x00, 0xCE, 0xD1 } },
    { "darkviolet", { 0x94, 0x00, 0xD3 } },
    { "deeppink", { 0xFF, 0x14, 0x93 } },
    { "deepskyblue", { 0x00, 0xBF, 0xFF } },
    { "dimgray", { 0x69, 0x69, 0x69 } },
    { "dodgerblue", { 0x1E, 0x90, 0xFF } },
    { "firebrick", { 0xB2, 0x22, 0x22 } },
    { "floralwhite", { 0xFF, 0xFA, 0xF0 } },
    { "forestgreen", { 0x22, 0x8B, 0x22 } },
    { "gainsboro", { 0xDC, 0xDC, 0xDC } },
    { "ghostwhite", { 0xF8, 0xF8, 0xFF } },
    { "gold", { 0xFF, 0xD7, 0x00 } },
    { "goldenrod", { 0xDA, 0xA5, 0x20 } },
    { "gray", { 0x80, 0x80, 0x80 } },
    { "green", { 0x00, 0x80, 0x00 } },
    { "greenyellow", { 0xAD, 0xFF, 0x2F } },
    { "honeydew", { 0xF0, 0xFF, 0xF0 } },
    { "hotpink", { 0xFF, 0x69, 0xB4 } },
    { "indianred", { 0xCD, 0x5C, 0x5C } },
    { "indigo", { 0x4B, 0x00, 0x82 } },
    { "ivory", { 0xFF, 0xFF, 0xF0 } },
    { "khaki", { 0xF0, 0xE6, 0x8C } },
    { "lavender", { 0xE6, 0xE6, 0xFA } },
    { "lavenderblush", { 0xFF, 0xF0, 0xF5 } },
    { "lawngreen", { 0x7C, 0xFC, 0x00 } },
    { "lemonchiffon", { 0xFF, 0xFA, 0xCD } },
    { "lightblue", { 0xAD, 0xD8, 0xE6 } },
    { "lightcoral", { 0xF0, 0x80, 0x80 } },
    { "lightcyan", { 0xE0, 0xFF, 0xFF } },
    { "lightgoldenrodyellow", { 0xFA, 0xFA, 0xD2 } },
    { "lightgray", { 0xD3, 0xD3, 0xD3 } },
    { "lightgreen", { 0x90, 0xEE, 0x90 } },
    { "lightpink", { 0xFF, 0xB6, 0xC1 } },
    { "lightsalmon", { 0xFF, 0xA0, 0x7A } },
    { "lightseagreen", { 0x20, 0xB2, 0xAA } },
    { "lightskyblue", { 0x87, 0xCE, 0xFA } },
    { "lightslategray", { 0x77, 0x88, 0x99 } },
    { "lightsteelblue", { 0xB0, 0xC4, 0xDE } },
    { "lightyellow", { 0xFF, 0xFF, 0xE0 } },
    { "lime", { 0x00, 0xFF, 0x00 } },
    { "limegreen", { 0x32, 0xCD, 0x32 } },
    { "linen", { 0xFA, 0xF0, 0xE6 } },
    { "magenta", { 0xFF, 0x00, 0xFF } },
    { "maroon", { 0x80, 0x00, 0x00 } },
    { "mediumaquamarine", { 0x66, 0xCD, 0xAA } },
    { "mediumblue", { 0x00, 0x00, 0xCD } },
    { "mediumorchid", { 0xBA, 0x55, 0xD3 } },
    { "mediumpurple", { 0x93, 0x70, 0xDB } },
    { "mediumseagreen", { 0x3C, 0xB3, 0x71 } },
    { "mediumslateblue", { 0x7B, 0x68, 0xEE } },
    { "mediumspringgreen", { 0x00, 0xFA, 0x9A } },
    { "mediumturquoise", { 0x48, 0xD1, 0xCC } },
    { "mediumvioletred", { 0xC7, 0x15, 0x85 } },
    { "midnightblue", { 0x19, 0x19, 0x70 } },
    { "mintcream", { 0xF5, 0xFF, 0xFA } },
    { "mistyrose", { 0xFF, 0xE4, 0xE1 } },
    { "moccasin", { 0xFF, 0xE4, 0xB5 } },
    { "navajowhite", { 0xFF, 0xDE, 0xAD } },
    { "navy", { 0x00, 0x00, 0x80 } },
    { "oldlace", { 0xFD, 0xF5, 0xE6 } },
    { "olive", { 0x80, 0x80, 0x00 } },
    { "olivedrab", { 0x6B, 0x8E, 0x23 } },
    { "orange", { 0xFF, 0xA5, 0x00 } },
    { "orangered", { 0xFF, 0x45, 0x00 } },
    { "orchid", { 0xDA, 0x70, 0xD6 } },
    { "palegoldenrod", { 0xEE, 0xE8, 0xAA } },
    { "palegreen", { 0x98, 0xFB, 0x98 } },
    { "paleturquoise", { 0xAF, 0xEE, 0xEE } },
    { "palevioletred", { 0xDB, 0x70, 0x93 } },
    { "papayawhip", { 0xFF, 0xEF, 0xD5 } },
    { "peachpuff", { 0xFF, 0xDA, 0xB9 } },
    { "peru", { 0xCD, 0x85, 0x3F } },
    { "pink", { 0xFF, 0xC0, 0xCB } },
    { "plum", { 0xDD, 0xA0, 0xDD } },
    { "powderblue", { 0xB0, 0xE0, 0xE6 } },
    { "purple", { 0x80, 0x00, 0x80 } },
    { "red", { 0xFF, 0x00, 0x00 } },
    { "rosybrown", { 0xBC, 0x8F, 0x8F } },
    { "royalblue", { 0x41, 0x69, 0xE1 } },
    { "saddlebrown", { 0x8B, 0x45, 0x13 } },
    { "salmon", { 0xFA, 0x80, 0x72 } },
    { "sandybrown", { 0xF4, 0xA4, 0x60 } },
    { "seagreen", { 0x2E, 0x8B, 0x57 } },
    { "seashell", { 0xFF, 0xF5, 0xEE } },
    { "sienna", { 0xA0, 0x52, 0x2D } },
    { "silver", { 0xC0, 0xC0, 0xC0 } },
    { "skyblue", { 0x87, 0xCE, 0xEB } },
    { "slateblue", { 0x6A, 0x5A, 0xCD } },
    { "slategray", { 0x70, 0x80, 0x90 } },
    { "snow", { 0xFF, 0xFA, 0xFA } },
    { "springgreen", { 0x00, 0xFF, 0x7F } },
    { "steelblue", { 0x46, 0x82, 0xB4 } },
    { "tan", { 0xD2, 0xB4, 0x8C } },
    { "teal", { 0x00, 0x80, 0x80 } },
    { "thistle", { 0xD8, 0xBF, 0xD8 } },
    { "tomato", { 0xFF, 0x63, 0x47 } },
    { "turquoise", { 0x40, 0xE0, 0xD0 } },
    { "violet", { 0xEE, 0x82, 0xEE } },
    { "wheat", { 0xF5, 0xDE, 0xB3 } },
    { "white", { 0xFF, 0xFF, 0xFF } },
    { "whitesmoke", { 0xF5, 0xF5, 0xF5 } },
    { "yellow", { 0xFF, 0xFF, 0x00 } },
    { "yellowgreen", { 0x9A, 0xCD, 0x32 } },
};

const map_type& get()
{
    static map_type mt(entries, std::size(entries), { 0x00, 0x00, 0x00 });
    return mt;
}

} // namespace named_color

namespace formula_error_policy {

using map_type = mdds::sorted_string_map<formula_error_policy_t, mdds::string_view_map_entry>;

// Keys must be sorted.
constexpr map_type::entry entries[] = {
    { "fail", formula_error_policy_t::fail },
    { "skip", formula_error_policy_t::skip },
};

const map_type& get()
{
    static map_type mt(entries, std::size(entries), formula_error_policy_t::unknown);
    return mt;
}

}

class to_size_t
{
    std::size_t m_value;

public:
    template<typename T>
    to_size_t(T v) : m_value(static_cast<std::size_t>(v))
    {
        static_assert(std::is_enum_v<T>, "source value type must be enum!");
    }

    operator std::size_t() const
    {
        return m_value;
    }
};

std::ostream& write_name_for_pos(
    std::ostream& os, const std::string_view* names, std::size_t n_names, to_size_t pos)
{
    if (pos < n_names)
        os << names[pos];
    else
        os << "???";

    return os;
}

} // anonymous namespace

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

col_width_t get_default_column_width()
{
    return std::numeric_limits<col_width_t>::max();
}

row_height_t get_default_row_height()
{
    return std::numeric_limits<row_height_t>::max();
}

totals_row_function_t to_totals_row_function_enum(std::string_view s)
{
    return trf::get().find(s);
}

pivot_cache_group_by_t to_pivot_cache_group_by_enum(std::string_view s)
{
    return pc_group_by::get().find(s);
}

error_value_t to_error_value_enum(std::string_view s)
{
    return error_value::get().find(s);
}

color_rgb_t to_color_rgb(std::string_view s)
{
    const char* p = s.data();
    std::size_t n = s.size();

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
        os << "'" << std::string_view(p0, n0) << "' is not a valid RGB color string.";
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
            os << "'" << std::string_view(p0, n0) << "' is not a valid RGB color string.";
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

color_rgb_t to_color_rgb_from_name(std::string_view s)
{
    return named_colors::get().find(s);
}

formula_error_policy_t to_formula_error_policy(std::string_view s)
{
    return formula_error_policy::get().find(s);
}

std::ostream& operator<< (std::ostream& os, error_value_t ev)
{
    switch (ev)
    {
        case error_value_t::div0:
            os << error_value::entries[0].key;
            break;
        case error_value_t::na:
            os << error_value::entries[1].key;
            break;
        case error_value_t::name:
            os << error_value::entries[2].key;
            break;
        case error_value_t::null:
            os << error_value::entries[3].key;
            break;
        case error_value_t::num:
            os << error_value::entries[4].key;
            break;
        case error_value_t::ref:
            os << error_value::entries[5].key;
            break;
        case error_value_t::value:
            os << error_value::entries[6].key;
            break;
        case error_value_t::unknown:
        default:
            ;
    }
    return os;
}

std::ostream& operator<< (std::ostream& os, border_style_t border)
{
    static constexpr std::string_view names[] = {
        "unknown",
        "none",
        "solid",
        "dash_dot",
        "dash_dot_dot",
        "dashed",
        "dotted",
        "double_border",
        "hair",
        "medium",
        "medium_dash_dot",
        "medium_dash_dot_dot",
        "medium_dashed",
        "slant_dash_dot",
        "thick",
        "thin",
        "double_thin",
        "fine_dashed",
    };

    return write_name_for_pos(os, names, std::size(names), border);
}

std::ostream& operator<< (std::ostream& os, formula_grammar_t grammar)
{
    static constexpr std::string_view names[] = {
        "unknown",
        "xls_xml",
        "xlsx",
        "ods",
        "gnumeric"
    };

    return write_name_for_pos(os, names, std::size(names), grammar);
}

std::ostream& operator<< (std::ostream& os, underline_t uline)
{
    static constexpr std::string_view names[] = {
        "none",
        "single_line",
        "single_accounting",
        "double_line",
        "double_accounting",
        "dotted",
        "dash",
        "long_dash",
        "dot_dash",
        "dot_dot_dash",
        "wave",
    };

    return write_name_for_pos(os, names, std::size(names), uline);
}

std::ostream& operator<< (std::ostream& os, underline_width_t ulwidth)
{
    static constexpr std::string_view names[] = {
        "none",
        "automatic",
        "bold",
        "dash",
        "medium",
        "thick",
        "thin",
        "percent",
        "positive_integer",
        "positive_length",
    };

    return write_name_for_pos(os, names, std::size(names), ulwidth);
}

std::ostream& operator<< (std::ostream& os, underline_mode_t ulmode)
{
    static constexpr std::string_view names[] = {
        "continuous",
        "skip_white_space",
    };

    return write_name_for_pos(os, names, std::size(names), ulmode);
}

std::ostream& operator<< (std::ostream& os, underline_type_t ultype)
{
    static constexpr std::string_view names[] = {
        "none",
        "single_type",
        "double_type",
    };

    return write_name_for_pos(os, names, std::size(names), ultype);
}

std::ostream& operator<< (std::ostream& os, hor_alignment_t halign)
{
    static constexpr std::string_view names[] = {
        "unknown",
        "left",
        "center",
        "right",
        "justified",
        "distributed",
        "filled",
    };

    return write_name_for_pos(os, names, std::size(names), halign);
}

std::ostream& operator<< (std::ostream& os, ver_alignment_t valign)
{
    static constexpr std::string_view names[] = {
        "unknown",
        "top",
        "middle",
        "bottom",
        "justified",
        "distributed",
    };

    return write_name_for_pos(os, names, std::size(names), valign);
}

std::ostream& operator<< (std::ostream& os, const color_rgb_t& color)
{
    os << "(r=" << (int)color.red << ",g=" << (int)color.green << ",b=" << (int)color.blue << ")";
    return os;
}

std::ostream& operator<< (std::ostream& os, const fill_pattern_t& fill)
{
    static constexpr std::string_view names[] = {
        "none",
        "solid",
        "dark_down",
        "dark_gray",
        "dark_grid",
        "dark_horizontal",
        "dark_trellis",
        "dark_up",
        "dark_vertical",
        "gray_0625",
        "gray_125",
        "light_down",
        "light_gray",
        "light_grid",
        "light_horizontal",
        "light_trellis",
        "light_up",
        "light_vertical",
        "medium_gray",
    };

    return write_name_for_pos(os, names, std::size(names), fill);
}

std::ostream& operator<< (std::ostream& os, const strikethrough_style_t& ss)
{
    static constexpr std::string_view names[] = {
        "none",
        "solid",
        "dash",
        "dot_dash",
        "dot_dot_dash",
        "dotted",
        "long_dash",
        "wave",
    };

    return write_name_for_pos(os, names, std::size(names), ss);
}

std::ostream& operator<< (std::ostream& os, const strikethrough_type_t& st)
{
    static constexpr std::string_view names[] = {
        "unknown",
        "none",
        "single_type",
        "double_type",
    };

    return write_name_for_pos(os, names, std::size(names), st);
}

std::ostream& operator<< (std::ostream& os, const strikethrough_width_t& sw)
{
    static constexpr std::string_view names[] = {
        "unknown",
        "width_auto",
        "thin",
        "medium",
        "thick",
        "bold",
    };

    return write_name_for_pos(os, names, std::size(names), sw);
}

std::ostream& operator<< (std::ostream& os, const strikethrough_text_t& st)
{
    static constexpr std::string_view names[] = {
        "unknown",
        "slash",
        "cross",
    };

    return write_name_for_pos(os, names, std::size(names), st);
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
