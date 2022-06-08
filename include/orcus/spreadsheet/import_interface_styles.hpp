/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <cstdlib>

#include "types.hpp"
#include "../types.hpp"
#include "../env.hpp"

// NB: This header must not depend on ixion, as it needs to be usable for
// those clients that provide their own formula engine.  Other headers in
// the orcus::spreadsheet namespace may depend on ixion.

namespace orcus { namespace spreadsheet { namespace iface {

class import_font_style;
class import_fill_style;
class import_border_style;
class import_cell_protection;
class import_number_format;
class import_xf;

/**
 * Interface for styles. Note that because the default style must have an
 * index of 0 in each style category, the caller must commit the default
 * styles first before importing and setting other, non-default styles. The
 * indices of the styles are assigned sequentially starting with 0 and upward
 * in each style category.  Font, fill, border, cell protection etc are
 * considered categories in this context.
 *
 * The term 'xf' here stands for cell format.
 *
 * In contrast to xf formatting, dxf (differential formats) format only stores
 * the format information that is different from the base format data.
 */
class ORCUS_DLLPUBLIC import_styles
{
public:
    virtual ~import_styles();

    /**
     * Return a pointer to the interface instance for importing font style
     * attributes. Note that the import_styles implementer <i>must</i> return a
     * non-null pointer.
     *
     * @return pointer to the interface instance for importing font style
     *         attributes.
     */
    virtual import_font_style* get_font_style() = 0;

    /**
     * Return a pointer to the interface instance for importing fill style
     * attributes. Note that the import_styles implementer <i>must</i> return a
     * non-null pointer.
     *
     * @return pointer to the interface instance for importing fill style
     *         attributes.
     */
    virtual import_fill_style* get_fill_style() = 0;

    /**
     * Return a pointer to the interface instance for importing border style
     * attributes. Note that the import_styles implementer <i>must</i> return a
     * non-null pointer.
     *
     * @return pointer to the interface instance for importing border style
     *         attributes.
     */
    virtual import_border_style* get_border_style() = 0;

    /**
     * Return a pointer to the interface instance for importing cell protection
     * attributes. Note that the import_styles implementer <i>must</i> return a
     * non-null pointer.
     *
     * @return pointer to the interface instance for importing cell protection
     *         attributes.
     */
    virtual import_cell_protection* get_cell_protection() = 0;

    /**
     * Return a pointer to the interface instance for importing number format
     * attributes. Note that the import_styles implementer <i>must</i> return a
     * non-null pointer.
     *
     * @return pointer to the interface instance for importing number format
     *         attributes.
     */
    virtual import_number_format* get_number_format() = 0;

    /**
     * Return a pointer to the interface instance for importing cell format
     * (xf) indices that each references different format attributes in their
     * respective pools.  Note that the import_styles implementer <i>must</i>
     * return a non-null pointer.
     *
     * @return pointer to the interface instance for importing cell format (xf)
     *         indices.
     */
    virtual import_xf* get_xf(xf_category_t cat) = 0;

    /**
     * Set the total number of font styles. This may be called before importing
     * any of the font styles. This will give the implementer a chance to
     * allocate storage.  Note that it may not always be called.
     *
     * @param n number of font styles.
     */
    virtual void set_font_count(size_t n) = 0;

    /**
     * Set the total number of fill styles. This may be called before importing
     * any of the fill styles. This will give the implementer a chance to
     * allocate storage.  Note that it may not always be called.
     *
     * @param n number of fill styles.
     */
    virtual void set_fill_count(size_t n) = 0;

    /**
     * Set the total number of border styles. This may be called before
     * importing any of the border styles. This will give the implementer a
     * chance to allocate storage.  Note that it may not always be called.
     *
     * @param n number of border styles.
     */
    virtual void set_border_count(size_t n) = 0;

    /**
     * Set the total number of number format styles. This may be called before
     * importing any of the number format styles. This will give the implementer
     * a chance to allocate storage.  Note that it may not always be called.
     *
     * @param n number of number format styles.
     */
    virtual void set_number_format_count(size_t n) = 0;

    // Cell format and cell style format, and differential cell format records.
    // All of these records share the same data structure for their entries. A
    // cell in a worksheet references an entry in the cell format record
    // directly by the index, and the entry in the cell format record references
    // a cell style format in the cell style format record by the index.

    virtual void set_xf_count(xf_category_t cat, size_t n) = 0;

    // named cell style. It references the actual cell format data via xf index
    // into the cell style format record.

    virtual void set_cell_style_count(size_t n) = 0;
    virtual void set_cell_style_name(std::string_view s) = 0;

    /**
     * Set the index into the cell style record.  The named cell style uses the
     * entry specified by this index as its format.
     *
     * @param index index into the cell style record.
     */
    virtual void set_cell_style_xf(size_t index) = 0;

    /**
     * Set the index into the built-in style record.
     *
     * @note This is Excel-specific, and unclear whether it's useful outside of
     * Excel's implementation.  Built-in styles are not stored in file.
     *
     * @param index index into the built-in cell style record.
     */
    virtual void set_cell_style_builtin(size_t index) = 0;

    /**
     * Set the name of the parent cell style it uses as its basis.
     *
     * @note ODF uses this but Excel does not use this value.
     *
     * @param s name of the parent cell style.
     */
    virtual void set_cell_style_parent_name(std::string_view s) = 0;
    virtual size_t commit_cell_style() = 0;
};

class ORCUS_DLLPUBLIC import_font_style
{
public:
    virtual ~import_font_style();

    virtual void set_bold(bool b) = 0;
    virtual void set_italic(bool b) = 0;
    virtual void set_name(std::string_view s) = 0;
    virtual void set_size(double point) = 0;
    virtual void set_underline(underline_t e) = 0;
    virtual void set_underline_width(underline_width_t e) = 0;
    virtual void set_underline_mode(underline_mode_t e) = 0;
    virtual void set_underline_type(underline_type_t e) = 0;
    virtual void set_underline_color(color_elem_t alpha, color_elem_t red, color_elem_t green, color_elem_t blue) = 0;
    virtual void set_color(color_elem_t alpha, color_elem_t red, color_elem_t green, color_elem_t blue) = 0;
    virtual void set_strikethrough_style(strikethrough_style_t s) = 0;
    virtual void set_strikethrough_type(strikethrough_type_t s) = 0;
    virtual void set_strikethrough_width(strikethrough_width_t s) = 0;
    virtual void set_strikethrough_text(strikethrough_text_t s) = 0;
    virtual size_t commit() = 0;
};

class ORCUS_DLLPUBLIC import_fill_style
{
public:
    virtual ~import_fill_style();

    /**
     * Set the type of fill pattern.
     *
     * @param fp fill pattern type.
     */
    virtual void set_pattern_type(fill_pattern_t fp) = 0;

    /**
     * Set the foreground color of a fill.  <i>Note that for a solid fill
     * type, the foreground color will be used.</i>
     *
     * @param alpha alpha component ranging from 0 (fully transparent) to 255
     *              (fully opaque).
     * @param red red component ranging from 0 to 255.
     * @param green green component ranging from 0 to 255.
     * @param blue blue component ranging from 0 to 255.
     */
    virtual void set_fg_color(color_elem_t alpha, color_elem_t red, color_elem_t green, color_elem_t blue) = 0;

    /**
     * Set the background color of a fill.  <i>Note that this color will
     * be ignored for a solid fill type.</i>
     *
     * @param alpha alpha component ranging from 0 (fully transparent) to 255
     *              (fully opaque).
     * @param red red component ranging from 0 to 255.
     * @param green green component ranging from 0 to 255.
     * @param blue blue component ranging from 0 to 255.
     */
    virtual void set_bg_color(color_elem_t alpha, color_elem_t red, color_elem_t green, color_elem_t blue) = 0;

    /**
     * Commit the fill style currently in the buffer.
     *
     * @return the ID of the committed fill style, to be passed on to the
     *         set_xf_fill() method as its argument.
     */
    virtual size_t commit() = 0;
};

class ORCUS_DLLPUBLIC import_border_style
{
public:
    virtual ~import_border_style();

    virtual void set_style(border_direction_t dir, border_style_t style) = 0;
    virtual void set_color(
        border_direction_t dir, color_elem_t alpha, color_elem_t red, color_elem_t green, color_elem_t blue) = 0;
    virtual void set_width(border_direction_t dir, double width, orcus::length_unit_t unit) = 0;
    virtual size_t commit() = 0;
};

class ORCUS_DLLPUBLIC import_cell_protection
{
public:
    virtual ~import_cell_protection();

    virtual void set_hidden(bool b) = 0;
    virtual void set_locked(bool b) = 0;
    virtual void set_print_content(bool b) = 0;
    virtual void set_formula_hidden(bool b) = 0;
    virtual size_t commit() = 0;
};

class ORCUS_DLLPUBLIC import_number_format
{
public:
    virtual ~import_number_format();

    virtual void set_identifier(std::size_t id) = 0;
    virtual void set_code(std::string_view s) = 0;
    virtual size_t commit() = 0;
};

class ORCUS_DLLPUBLIC import_xf
{
public:
    virtual ~import_xf();

    virtual void set_font(size_t index) = 0;
    virtual void set_fill(size_t index) = 0;
    virtual void set_border(size_t index) = 0;
    virtual void set_protection(size_t index) = 0;
    virtual void set_number_format(size_t index) = 0;

    /**
     * Set the index into the cell style record to specify a named cell style it
     * uses as its basis.
     *
     * @param index index into the cell style record it uses as its basis.
     */
    virtual void set_style_xf(size_t index) = 0;
    virtual void set_apply_alignment(bool b) = 0;
    virtual void set_horizontal_alignment(hor_alignment_t align) = 0;
    virtual void set_vertical_alignment(ver_alignment_t align) = 0;

    virtual size_t commit() = 0;
};

}}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
