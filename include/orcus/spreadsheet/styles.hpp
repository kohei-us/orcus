/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_ORCUS_SPREADSHEET_STYLES_HPP
#define INCLUDED_ORCUS_SPREADSHEET_STYLES_HPP

#include "../env.hpp"
#include "../measurement.hpp"
#include "types.hpp"

#include <memory>
#include <string_view>

namespace orcus { namespace spreadsheet {

struct ORCUS_SPM_DLLPUBLIC color_t
{
    color_elem_t alpha;
    color_elem_t red;
    color_elem_t green;
    color_elem_t blue;

    color_t();
    color_t(color_elem_t _red, color_elem_t _green, color_elem_t _blue);
    color_t(color_elem_t _alpha, color_elem_t _red, color_elem_t _green, color_elem_t _blue);

    void reset();

    bool operator==(const color_t& other) const;
    bool operator!=(const color_t& other) const;
};

struct ORCUS_SPM_DLLPUBLIC font_t
{
    std::string_view name;
    double size;
    bool bold:1;
    bool italic:1;
    underline_t underline_style;
    underline_width_t underline_width;
    underline_mode_t underline_mode;
    underline_type_t underline_type;
    color_t underline_color;
    color_t color;
    strikethrough_style_t strikethrough_style;
    strikethrough_width_t strikethrough_width;
    strikethrough_type_t strikethrough_type;
    strikethrough_text_t strikethrough_text;

    font_t();
    void reset();
};

/**
 * Specifies whether each attribute of font_t is active or not.
 */
struct ORCUS_SPM_DLLPUBLIC font_active_t
{
    bool name = false;
    bool size = false;
    bool bold = false;
    bool italic = false;
    bool underline_style = false;
    bool underline_width = false;
    bool underline_mode = false;
    bool underline_type = false;
    bool underline_color = false;
    bool color = false;
    bool strikethrough_style = false;
    bool strikethrough_width = false;
    bool strikethrough_type = false;
    bool strikethrough_text = false;

    void set() noexcept;
    void reset();

    bool operator== (const font_active_t& other) const noexcept;
    bool operator!= (const font_active_t& other) const noexcept;
};

struct ORCUS_SPM_DLLPUBLIC fill_t
{
    fill_pattern_t pattern_type;
    color_t fg_color;
    color_t bg_color;

    fill_t();
    void reset();
};

/**
 * Specifies whether each attribute of fill_t is active or not.
 */
struct ORCUS_SPM_DLLPUBLIC fill_active_t
{
    bool pattern_type = false;
    bool fg_color = false;
    bool bg_color = false;

    void set() noexcept;
    void reset();

    bool operator== (const fill_active_t& other) const noexcept;
    bool operator!= (const fill_active_t& other) const noexcept;
};

struct ORCUS_SPM_DLLPUBLIC border_attrs_t
{
    border_style_t style;
    color_t border_color;
    length_t border_width;

    border_attrs_t();
    void reset();
};

struct ORCUS_SPM_DLLPUBLIC border_t
{
    border_attrs_t top;
    border_attrs_t bottom;
    border_attrs_t left;
    border_attrs_t right;
    border_attrs_t diagonal;
    border_attrs_t diagonal_bl_tr;
    border_attrs_t diagonal_tl_br;

    border_t();
    void reset();
};

struct ORCUS_SPM_DLLPUBLIC protection_t
{
    bool locked;
    bool hidden;
    bool print_content;
    bool formula_hidden;

    protection_t();
    void reset();
};

struct ORCUS_SPM_DLLPUBLIC number_format_t
{
    size_t identifier;
    std::string_view format_string;

    number_format_t();
    void reset();
    bool operator== (const number_format_t& r) const;
};

/**
 * Cell format attributes
 */
struct ORCUS_SPM_DLLPUBLIC cell_format_t
{
    size_t font;            /// font ID
    size_t fill;            /// fill ID
    size_t border;          /// border ID
    size_t protection;      /// protection ID
    size_t number_format;   /// number format ID
    size_t style_xf;        /// style XF ID (used only for cell format)
    hor_alignment_t hor_align;
    ver_alignment_t ver_align;
    bool apply_num_format:1;
    bool apply_font:1;
    bool apply_fill:1;
    bool apply_border:1;
    bool apply_alignment:1;
    bool apply_protection:1;

    cell_format_t();
    void reset();
};

struct ORCUS_SPM_DLLPUBLIC cell_style_t
{
    std::string_view name;
    size_t xf;
    size_t builtin;
    std::string_view parent_name;

    cell_style_t();
    void reset();
};

ORCUS_SPM_DLLPUBLIC std::ostream& operator<< (std::ostream& os, const color_t& c);

class ORCUS_SPM_DLLPUBLIC styles
{
    struct impl;
    std::unique_ptr<impl> mp_impl;

public:
    styles();
    ~styles();

    void reserve_font_store(size_t n);
    size_t append_font(const font_t& font);
    size_t append_font(const font_t& value, const font_active_t& active);

    void reserve_fill_store(size_t n);
    size_t append_fill(const fill_t& fill);
    size_t append_fill(const fill_t& value, const fill_active_t& active);

    void reserve_border_store(size_t n);
    size_t append_border(const border_t& border);

    size_t append_protection(const protection_t& protection);

    void reserve_number_format_store(size_t n);
    size_t append_number_format(const number_format_t& nf);

    void reserve_cell_style_format_store(size_t n);
    size_t append_cell_style_format(const cell_format_t& cf);

    void reserve_cell_format_store(size_t n);
    size_t append_cell_format(const cell_format_t& cf);

    void reserve_diff_cell_format_store(size_t n);
    size_t append_diff_cell_format(const cell_format_t& cf);

    void reserve_cell_style_store(size_t n);
    size_t append_cell_style(const cell_style_t& cs);

    const font_t* get_font(size_t index) const;
    const std::pair<font_t, font_active_t>* get_font_state(size_t index) const;

    const fill_t* get_fill(size_t index) const;
    const std::pair<fill_t, fill_active_t>* get_fill_state(size_t index) const;

    const border_t* get_border(size_t index) const;
    const protection_t* get_protection(size_t index) const;
    const number_format_t* get_number_format(size_t index) const;
    const cell_format_t* get_cell_format(size_t index) const;
    const cell_format_t* get_cell_style_format(size_t index) const;
    const cell_format_t* get_dxf_format(size_t index) const;
    const cell_style_t* get_cell_style(size_t index) const;

    size_t get_font_count() const;
    size_t get_fill_count() const;
    size_t get_border_count() const;
    size_t get_protection_count() const;
    size_t get_number_format_count() const;
    size_t get_cell_formats_count() const;
    size_t get_cell_style_formats_count() const;
    size_t get_dxf_count() const;
    size_t get_cell_styles_count() const;

    void clear();
};

}}

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
