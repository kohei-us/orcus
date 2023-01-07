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
#include "document_types.hpp"

#include <memory>
#include <string_view>
#include <optional>

namespace orcus { namespace spreadsheet {

class document;

struct ORCUS_SPM_DLLPUBLIC font_t
{
    std::optional<std::string_view> name;
    std::optional<std::string_view> name_asian;
    std::optional<std::string_view> name_complex;
    std::optional<double> size;
    std::optional<double> size_asian;
    std::optional<double> size_complex;
    std::optional<bool> bold;
    std::optional<bool> bold_asian;
    std::optional<bool> bold_complex;
    std::optional<bool> italic;
    std::optional<bool> italic_asian;
    std::optional<bool> italic_complex;
    std::optional<underline_t> underline_style;
    std::optional<underline_width_t> underline_width;
    std::optional<underline_mode_t> underline_mode;
    std::optional<underline_type_t> underline_type;
    std::optional<color_t> underline_color;
    std::optional<color_t> color;
    std::optional<strikethrough_style_t> strikethrough_style;
    std::optional<strikethrough_width_t> strikethrough_width;
    std::optional<strikethrough_type_t> strikethrough_type;
    std::optional<strikethrough_text_t> strikethrough_text;

    font_t();
    void reset();
};

struct ORCUS_SPM_DLLPUBLIC fill_t
{
    std::optional<fill_pattern_t> pattern_type;
    std::optional<color_t> fg_color;
    std::optional<color_t> bg_color;

    fill_t();
    void reset();
};

struct ORCUS_SPM_DLLPUBLIC border_attrs_t
{
    std::optional<border_style_t> style;
    std::optional<color_t> border_color;
    std::optional<length_t> border_width;

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
    std::optional<bool> locked;
    std::optional<bool> hidden;
    std::optional<bool> print_content;
    std::optional<bool> formula_hidden;

    protection_t();
    void reset();
};

struct ORCUS_SPM_DLLPUBLIC number_format_t
{
    std::optional<std::size_t> identifier;
    std::optional<std::string_view> format_string;

    number_format_t();
    void reset();

    bool operator== (const number_format_t& other) const noexcept;
    bool operator!= (const number_format_t& other) const noexcept;
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
    std::optional<bool> wrap_text;
    std::optional<bool> shrink_to_fit;
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
    std::string_view display_name;
    size_t xf;
    size_t builtin;
    std::string_view parent_name;

    cell_style_t();
    void reset();
};

ORCUS_SPM_DLLPUBLIC std::ostream& operator<< (std::ostream& os, const color_t& c);

class ORCUS_SPM_DLLPUBLIC styles
{
    friend class document;

    struct impl;
    std::unique_ptr<impl> mp_impl;

public:
    styles();
    ~styles();

    void reserve_font_store(size_t n);
    std::size_t append_font(const font_t& font);

    void reserve_fill_store(size_t n);
    std::size_t append_fill(const fill_t& fill);

    void reserve_border_store(size_t n);
    std::size_t append_border(const border_t& border);

    std::size_t append_protection(const protection_t& protection);

    void reserve_number_format_store(size_t n);
    std::size_t append_number_format(const number_format_t& nf);

    void reserve_cell_style_format_store(size_t n);
    size_t append_cell_style_format(const cell_format_t& cf);

    void reserve_cell_format_store(size_t n);
    size_t append_cell_format(const cell_format_t& cf);

    void reserve_diff_cell_format_store(size_t n);
    size_t append_diff_cell_format(const cell_format_t& cf);

    void reserve_cell_style_store(size_t n);
    void append_cell_style(const cell_style_t& cs);

    const font_t* get_font(size_t index) const;
    const fill_t* get_fill(size_t index) const;
    const border_t* get_border(size_t index) const;
    const protection_t* get_protection(size_t index) const;
    const number_format_t* get_number_format(size_t index) const;
    const cell_format_t* get_cell_format(size_t index) const;
    const cell_format_t* get_cell_style_format(size_t index) const;
    const cell_format_t* get_dxf_format(size_t index) const;
    const cell_style_t* get_cell_style(size_t index) const;
    const cell_style_t* get_cell_style_by_xf(size_t xfid) const;

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

private:
    void finalize_import();
};

}}

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
