/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "odf_styles.hpp"

#include <stdexcept>

namespace orcus {

odf_style::odf_style() : family(style_family_unknown) {}
odf_style::odf_style(std::string_view _name, odf_style_family _family, std::string_view parent) :
    name(_name),
    family(_family),
    parent_name(parent)
{
    switch (family)
    {
        case style_family_table_column:
            data = column{};
            break;
        case style_family_table_row:
            data = row{};
            break;
        case style_family_table_cell:
            data = cell{};
            break;
        case style_family_table:
            data = table{};
            break;
        case style_family_graphic:
            data = graphic{};
            break;
        case style_family_paragraph:
            data = paragraph{};
            break;
        case style_family_text:
            data = text{};
            break;
        case style_family_unknown:
            throw std::invalid_argument("unkown style family is not allowed");
    }
}

odf_style::~odf_style() {}

number_formatting_style::number_formatting_style(std::string_view style_name, const bool volatile_style):
    number_formatting(0),
    name(style_name),
    is_volatile(volatile_style)
{
}


}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
