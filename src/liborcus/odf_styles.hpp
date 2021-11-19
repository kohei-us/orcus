/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_ORCUS_ODF_STYLES_HPP
#define INCLUDED_ORCUS_ODF_STYLES_HPP

#include <orcus/measurement.hpp>

#include <map>
#include <memory>

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
        size_t font;
        size_t fill;
        size_t border;
        size_t protection;

        size_t xf;
        bool automatic_style;

        cell() : font(0), fill(0), border(0), protection(0),
                xf(0), automatic_style(false) {}
    };

    struct table
    {
    };

    struct graphic
    {
    };

    struct paragraph
    {
    };

    struct text
    {
        size_t font;
    };

    std::string_view name;
    odf_style_family family;
    std::string_view parent_name;

    union {
        column* column_data;
        row* row_data;
        table* table_data;
        cell* cell_data;
        graphic* graphic_data;
        paragraph* paragraph_data;
        text* text_data;
    };

    odf_style(const odf_style&) = delete;
    odf_style& operator=(const odf_style&) = delete;

    odf_style();
    odf_style(std::string_view _name, odf_style_family _family, std::string_view parent);

    ~odf_style();
};

struct number_formatting_style
{
    size_t number_formatting;

    std::string_view name;
    std::string number_formatting_code;
    bool is_volatile;
    std::string_view character_stream;

    number_formatting_style():
        number_formatting(0),
        is_volatile(0)
        {}

    number_formatting_style(std::string_view style_name, const bool volatile_style);
};

typedef std::map<std::string_view, std::unique_ptr<odf_style>> odf_styles_map_type;

}

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
