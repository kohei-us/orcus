/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_ORCUS_ODF_HELPER_HPP
#define INCLUDED_ORCUS_ODF_HELPER_HPP

#include <orcus/pstring.hpp>
#include <orcus/spreadsheet/types.hpp>
#include <orcus/measurement.hpp>
#include <orcus/spreadsheet/styles.hpp>

namespace orcus {

class odf_helper
{
public:
    struct odf_border_details
    {
        orcus::spreadsheet::border_style_t border_style;

        spreadsheet::color_elem_t red;
        spreadsheet::color_elem_t green;
        spreadsheet::color_elem_t blue;

        length_t border_width;

        odf_border_details():
            border_style(orcus::spreadsheet::border_style_t::unknown),
            red(0),
            green(0),
            blue(0)
            {}
    };

    static bool convert_fo_color(const orcus::pstring& value, orcus::spreadsheet::color_elem_t& red,
            orcus::spreadsheet::color_elem_t& green, orcus::spreadsheet::color_elem_t& blue);

    /**
     * extracts border style, width and colors out of the pstring provided to it
    **/
    static orcus::odf_helper::odf_border_details extract_border_details(const orcus::pstring& value);

    static orcus::spreadsheet::underline_width_t extract_underline_width(const orcus::pstring& value);

    static orcus::spreadsheet::underline_t extract_underline_style(const orcus::pstring& value);

    static bool extract_hor_alignment_style(const orcus::pstring& value,
            spreadsheet::hor_alignment_t& alignment);

    static bool extract_ver_alignment_style(const orcus::pstring& value,
            spreadsheet::ver_alignment_t& alignment);

};

}

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
