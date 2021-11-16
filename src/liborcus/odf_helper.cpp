/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "odf_helper.hpp"
#include "string_helper.hpp"
#include <orcus/spreadsheet/types.hpp>
#include <orcus/measurement.hpp>
#include <mdds/sorted_string_map.hpp>
#include <mdds/global.hpp>
#include <orcus/global.hpp>
#include <orcus/spreadsheet/styles.hpp>

namespace ss = orcus::spreadsheet;

namespace orcus {

namespace {

typedef mdds::sorted_string_map<spreadsheet::border_style_t> odf_border_style_map;

odf_border_style_map::entry odf_border_style_entries[] =
{
    { MDDS_ASCII("dash-dot"), spreadsheet::border_style_t::dash_dot},
    { MDDS_ASCII("dash-dot-dot"), spreadsheet::border_style_t::dash_dot_dot},
    { MDDS_ASCII("dashed"), spreadsheet::border_style_t::dashed},
    { MDDS_ASCII("dotted"), spreadsheet::border_style_t::dotted},
    { MDDS_ASCII("double-thin"), spreadsheet::border_style_t::double_thin},
    { MDDS_ASCII("fine-dashed"), spreadsheet::border_style_t::fine_dashed},
    { MDDS_ASCII("none"), spreadsheet::border_style_t::none},
    { MDDS_ASCII("solid"), spreadsheet::border_style_t::solid},
    { MDDS_ASCII("unknown"), spreadsheet::border_style_t::unknown}
};

typedef mdds::sorted_string_map<spreadsheet::underline_width_t> odf_underline_width_map;

odf_underline_width_map::entry odf_underline_width_entries[] =
{
    { MDDS_ASCII("bold"), spreadsheet::underline_width_t::bold},
    { MDDS_ASCII("medium"), spreadsheet::underline_width_t::medium},
    { MDDS_ASCII("none"), spreadsheet::underline_width_t::none},
    { MDDS_ASCII("normal"), spreadsheet::underline_width_t::normal},
    { MDDS_ASCII("percent"), spreadsheet::underline_width_t::percent},
    { MDDS_ASCII("positiveInteger"), spreadsheet::underline_width_t::positive_integer},
    { MDDS_ASCII("positiveLength"), spreadsheet::underline_width_t::positive_length},
    { MDDS_ASCII("thick"), spreadsheet::underline_width_t::thick},
    { MDDS_ASCII("thin"), spreadsheet::underline_width_t::thin},
};

namespace underline_style {

typedef mdds::sorted_string_map<ss::underline_t> map_type;

// Keys must be sorted.
map_type::entry entries[] =
{
    { MDDS_ASCII("dash"), ss::underline_t::dash },
    { MDDS_ASCII("dot-dash"), ss::underline_t::dot_dash },
    { MDDS_ASCII("dot-dot-dash"), ss::underline_t::dot_dot_dot_dash },
    { MDDS_ASCII("dotted"), ss::underline_t::dotted },
    { MDDS_ASCII("long-dash"), ss::underline_t::long_dash },
    { MDDS_ASCII("none"), ss::underline_t::none },
    { MDDS_ASCII("solid"), ss::underline_t::single_line },
    { MDDS_ASCII("wave"), ss::underline_t::wave }
};

const map_type& get()
{
    static map_type mt(entries, std::size(entries), ss::underline_t::none);
    return mt;
}

} // namespace underline_style

typedef mdds::sorted_string_map<spreadsheet::hor_alignment_t> odf_horizontal_alignment_map;

odf_horizontal_alignment_map::entry odf_horizontal_alignment_entries[] =
{
    { MDDS_ASCII("center"), spreadsheet::hor_alignment_t::center},
    { MDDS_ASCII("end"), spreadsheet::hor_alignment_t::right},
    { MDDS_ASCII("justified"), spreadsheet::hor_alignment_t::justified},
    { MDDS_ASCII("start"), spreadsheet::hor_alignment_t::left}
};

typedef mdds::sorted_string_map<spreadsheet::ver_alignment_t> odf_vertical_alignment_map;

odf_vertical_alignment_map::entry odf_vertical_alignment_entries[] =
{
    { MDDS_ASCII("bottom"), spreadsheet::ver_alignment_t::bottom},
    { MDDS_ASCII("justified"), spreadsheet::ver_alignment_t::justified},
    { MDDS_ASCII("middle"), spreadsheet::ver_alignment_t::middle},
    { MDDS_ASCII("top"), spreadsheet::ver_alignment_t::top}
};

bool is_valid_hex_digit(const char& character, orcus::spreadsheet::color_elem_t& val)
{
    if ('0' <= character && character <= '9')
    {
        val += character - '0';
        return true;
    }

    if ('A' <= character && character <= 'F')
    {
        val += character - 'A' + 10;
        return true;
    }

    if ('a' <= character && character <= 'f')
    {
        val += character - 'a' + 10;
        return true;
    }

    return false;
}

// converts two characters starting at index to a color value
bool convert_color_digits(const pstring& value, orcus::spreadsheet::color_elem_t& color_val, size_t index)
{
    const char& high_val = value[index];
    color_val = 0;
    if (!is_valid_hex_digit(high_val, color_val))
        return false;
    color_val *= 16;
    const char& low_val = value[++index];
    return is_valid_hex_digit(low_val, color_val);
}

}

bool odf::convert_fo_color(
    std::string_view value,
    spreadsheet::color_elem_t& red,
    spreadsheet::color_elem_t& green,
    spreadsheet::color_elem_t& blue)
{
    auto color = convert_fo_color(value);
    if (!color)
        return false;

    red = color->red;
    green = color->green;
    blue = color->blue;
    return true;
}

std::optional<spreadsheet::color_rgb_t> odf::convert_fo_color(std::string_view value)
{
    std::optional<spreadsheet::color_rgb_t> ret;

    // first character needs to be '#'
    if (value.size() != 7)
        return ret;

    if (value[0] != '#')
        return ret;

    spreadsheet::color_rgb_t color;
    if (!convert_color_digits(value, color.red, 1))
        return ret;

    if (!convert_color_digits(value, color.green, 3))
        return ret;

    if (!convert_color_digits(value, color.blue, 5))
        return ret;

    return color;
}

orcus::odf::border_details odf::extract_border_details(std::string_view value)
{
    border_details border_details;

    std::vector<pstring> detail = orcus::string_helper::split_string(value,' ');

    for (auto& sub_detail : detail)
    {
        if (sub_detail[0] == '#')
            convert_fo_color(sub_detail, border_details.red, border_details.green, border_details.blue);
        else if (sub_detail[0] >= '0' && sub_detail[0] <='9')
            border_details.border_width = orcus::to_length(sub_detail);
        else    //  This has to be a style
        {
            odf_border_style_map border_style_map(odf_border_style_entries, ORCUS_N_ELEMENTS(odf_border_style_entries), spreadsheet::border_style_t::none);
            border_details.border_style = border_style_map.find(sub_detail.get(), sub_detail.size());
        }

    }
    return border_details;
}

orcus::spreadsheet::underline_width_t odf::extract_underline_width(std::string_view value)
{
    orcus::spreadsheet::underline_width_t underline_width;

    odf_underline_width_map underline_width_map(odf_underline_width_entries, ORCUS_N_ELEMENTS(odf_underline_width_entries), spreadsheet::underline_width_t::none);
    underline_width = underline_width_map.find(value.data(), value.size());

    return underline_width;
}

orcus::spreadsheet::underline_t odf::extract_underline_style(std::string_view value)
{
    return underline_style::get().find(value.data(), value.size());
}

bool odf::extract_hor_alignment_style(std::string_view value, spreadsheet::hor_alignment_t& alignment)
{
    odf_horizontal_alignment_map horizontal_alignment_map(odf_horizontal_alignment_entries,
            ORCUS_N_ELEMENTS(odf_horizontal_alignment_entries),
            spreadsheet::hor_alignment_t::unknown);

    alignment = horizontal_alignment_map.find(value.data(), value.size());

    if (alignment == spreadsheet::hor_alignment_t::unknown)
        return false;

    return true;
}

bool odf::extract_ver_alignment_style(std::string_view value, spreadsheet::ver_alignment_t& alignment)
{
    odf_vertical_alignment_map vertical_alignment_map(odf_vertical_alignment_entries,
            ORCUS_N_ELEMENTS(odf_vertical_alignment_entries),
            spreadsheet::ver_alignment_t::unknown);
    alignment = vertical_alignment_map.find(value.data(), value.size());

    if (alignment == spreadsheet::ver_alignment_t::unknown)
        return false;

    return true;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
