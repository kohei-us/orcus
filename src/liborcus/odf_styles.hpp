/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_ORCUS_ODF_STYLES_HPP
#define INCLUDED_ORCUS_ODF_STYLES_HPP

#include <orcus/measurement.hpp>
#include <orcus/spreadsheet/types.hpp>

#include <map>
#include <memory>
#include <variant>
#include <ostream>
#include <optional>

namespace orcus {

enum odf_style_family
{
    style_family_unknown = 0,
    style_family_table_column,
    style_family_table_row,
    style_family_table_cell,
    style_family_table,
    style_family_graphic,
    style_family_paragraph,
    style_family_text
};

/**
 * Each instance of this class represents a single <style:style> entry.
 */
struct odf_style
{
    struct column
    {
        length_t width;
    };

    struct row
    {
        length_t height;
        bool height_set = false;
    };

    struct cell
    {
        std::size_t font = 0;
        std::size_t fill = 0;
        std::size_t border = 0;
        std::size_t protection = 0;
        std::size_t xf = 0;
        std::size_t number_format = 0;
        spreadsheet::hor_alignment_t hor_align = spreadsheet::hor_alignment_t::unknown;
        spreadsheet::ver_alignment_t ver_align = spreadsheet::ver_alignment_t::unknown;
        std::optional<bool> wrap_text;
        std::optional<bool> shrink_to_fit;
    };

    struct table
    {
    };

    struct graphic
    {
    };

    struct paragraph
    {
        spreadsheet::hor_alignment_t hor_align = spreadsheet::hor_alignment_t::unknown;
    };

    struct text
    {
        size_t font;
    };

    using data_type = std::variant<column, row, cell, table, graphic, paragraph, text>;

    std::string_view name;
    odf_style_family family;
    std::string_view parent_name;

    data_type data;

    odf_style(const odf_style&) = delete;
    odf_style& operator=(const odf_style&) = delete;

    odf_style();
    odf_style(std::string_view _name, odf_style_family _family, std::string_view parent);

    ~odf_style();
};

struct odf_number_format
{
    std::string_view name;
    std::string code;
    bool is_volatile = false;

    odf_number_format() = default;
    odf_number_format(std::string_view _name, bool _is_volatile);
};

using odf_styles_map_type = std::map<std::string_view, std::unique_ptr<odf_style>>;

/**
 * Merge two styles collections into one.
 *
 * @param dst destination where all the styles will be stored when the call
 *            returns.
 * @param src source collection to move all the styles from. After the call
 *            returns this one will be empty.
 */
void merge(odf_styles_map_type& dst, odf_styles_map_type& src);

void dump_state(const odf_styles_map_type& styles_map, std::ostream& os);

}

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
