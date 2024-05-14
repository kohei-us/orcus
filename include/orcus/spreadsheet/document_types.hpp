/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "types.hpp"
#include <vector>
#include <optional>

namespace orcus { namespace spreadsheet {

/**
 * Stores a color value in ARGB format.  Each color component ranges from 0 to
 * 255 (8-bit).
 */
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

struct ORCUS_SPM_DLLPUBLIC strikethrough_t
{
    std::optional<strikethrough_style_t> style;
    std::optional<strikethrough_type_t> type;
    std::optional<strikethrough_width_t> width;
    std::optional<strikethrough_text_t> text;

    strikethrough_t();
    strikethrough_t(const strikethrough_t& other);
    ~strikethrough_t();

    strikethrough_t& operator=(const strikethrough_t& other);

    bool operator==(const strikethrough_t& other) const;
    bool operator!=(const strikethrough_t& other) const;

    void reset();

    bool has_value() const;
};

/**
 * Contains formatting properties of a section of a string.  This is used in
 * the storage of rich-text strings.
 */
struct ORCUS_SPM_DLLPUBLIC format_run_t
{
    /** Position of the section where the formatting starts. */
    std::size_t pos = 0;
    /** Length of the section. */
    std::size_t size = 0;
    /** Name of the font. */
    std::optional<std::string_view> font;
    /** Size of the font. */
    std::optional<double> font_size;
    /** Color of the section. */
    std::optional<color_t> color;
    /** Whether or not the font is bold. */
    std::optional<bool> bold;
    /** Whether or not the font is italic. */
    std::optional<bool> italic;
    /** Whether or not the font has superscript applied. */
    std::optional<bool> superscript;
    /** Whether or not the font has subscript applied. */
    std::optional<bool> subscript;
    /** Strikethrough attributes. */
    strikethrough_t strikethrough;

    format_run_t();
    format_run_t(const format_run_t& other);
    ~format_run_t();

    format_run_t& operator=(const format_run_t& other);

    /**
     * Reset the properties to unformatted state.
     */
    void reset();

    /**
     * Query whether or not the section contains non-default format properties.
     *
     * @return @p true of it's formatted, otherwise @p false.
     */
    bool formatted() const;
};

/** Collection of format properties of a string. */
using format_runs_t = std::vector<format_run_t>;

}} // namespace orcus::spreadsheet

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
