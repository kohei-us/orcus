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

odf_number_format::odf_number_format(std::string_view _name, bool _is_volatile):
    name(_name),
    is_volatile(_is_volatile)
{
}

void dump_state(const odf_styles_map_type& styles_map, std::ostream& os)
{
    os << "styles picked up:\n";

    auto it = styles_map.begin(), it_end = styles_map.end();
    for (; it != it_end; ++it)
    {
        os << "  style: " << it->first << " [ ";

        switch (it->second->family)
        {
            case style_family_table_column:
            {
                const auto& data = std::get<odf_style::column>(it->second->data);
                os << "column width: " << data.width.to_string();
                break;
            }
            case style_family_table_row:
            {
                const auto& data = std::get<odf_style::row>(it->second->data);
                os << "row height: " << data.height.to_string();
                break;
            }
            case style_family_table_cell:
            {
                const auto& cell = std::get<odf_style::cell>(it->second->data);
                os << "xf ID: " << cell.xf;
                break;
            }
            case style_family_text:
            {
                const auto& data = std::get<odf_style::text>(it->second->data);
                os << "font ID: " << data.font;
                break;
            }
            default:
                ;
        }

        os << " ]\n";
    }
}

} // namespace orcus

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
