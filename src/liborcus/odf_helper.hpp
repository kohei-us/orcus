/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_ORCUS_ODF_HELPER_HPP
#define INCLUDED_ORCUS_ODF_HELPER_HPP

#include <orcus/spreadsheet/types.hpp>
#include <orcus/measurement.hpp>
#include <orcus/spreadsheet/styles.hpp>

#include <optional>

namespace orcus { namespace odf {

struct border_details_t
{
    spreadsheet::border_style_t border_style = spreadsheet::border_style_t::unknown;

    spreadsheet::color_elem_t red = 0;
    spreadsheet::color_elem_t green = 0;
    spreadsheet::color_elem_t blue = 0;

    length_t border_width;
};

bool convert_fo_color(
    std::string_view value,
    spreadsheet::color_elem_t& red,
    spreadsheet::color_elem_t& green,
    spreadsheet::color_elem_t& blue);

std::optional<spreadsheet::color_rgb_t> convert_fo_color(std::string_view value);

/**
 * extracts border style, width and colors from a string value.
 */
border_details_t extract_border_details(std::string_view value);

spreadsheet::underline_thickness_t extract_underline_width(std::string_view value);

spreadsheet::underline_t extract_underline_style(std::string_view value);

spreadsheet::hor_alignment_t extract_hor_alignment_style(std::string_view value);

spreadsheet::ver_alignment_t extract_ver_alignment_style(std::string_view value);

}}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
