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
#include <optional>

namespace orcus { namespace spreadsheet {

struct ORCUS_SPM_DLLPUBLIC font_t
{
    std::string_view name;
    std::string_view name_asian;
    std::string_view name_complex;
    double size;
    double size_asian;
    double size_complex;
    bool bold:1;
    bool bold_asian:1;
    bool bold_complex:1;
    bool italic:1;
    bool italic_asian:1;
    bool italic_complex:1;
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
    bool name_asian = false;
    bool name_complex = false;
    bool size = false;
    bool size_asian = false;
    bool size_complex = false;
    bool bold = false;
    bool bold_asian = false;
    bool bold_complex = false;
    bool italic = false;
    bool italic_asian = false;
    bool italic_complex = false;
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

namespace detail {

template<typename T>
struct to_active_type;

template<> struct to_active_type<font_t> { using type = font_active_t; };

} // namespace detail

/**
 * Template to pair the source style type with its active flags.  The active
 * flags store whether each style attribute is applied or not.
 */
template<typename T>
using style_attrs_t = std::pair<T, typename detail::to_active_type<T>::type>;

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
    const style_attrs_t<font_t>* get_font_state(size_t index) const;

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
    void finalize();
};

}}

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
