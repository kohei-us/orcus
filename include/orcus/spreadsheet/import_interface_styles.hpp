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
class import_cell_style;

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
     * Return a pointer to the interface instance for importing named cell style
     * information.
     *
     * @return pointer to the interface instance for importing named cell style
     *         information.
     */
    virtual import_cell_style* get_cell_style() = 0;

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

    /**
     * Set the total number of cell format styles for a specified cell format
     * category. This may be called before importing any of the cell format
     * styles for the specified category. This will give the implementer a
     * chance to allocate storage. Note that it may not always be called.
     *
     * @param cat cell format category.
     * @param n number of cell formats styles for the specified cell format
     *          category.
     */
    virtual void set_xf_count(xf_category_t cat, size_t n) = 0;

    /**
     * Set the total number of named cell styles.  This may be called before
     * importing any cell styles to give the implementer a chance to allocate
     * storage. Note that it may not always be called.
     *
     * @param n number of named cell styles.
     */
    virtual void set_cell_style_count(size_t n) = 0;
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

    /**
     * Commit the font style in the current buffer.
     *
     * @return index of the committed font style, to be passed on to the
     *         import_xf::set_font() method as its argument.
     */
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
     * Commit the fill style in the current buffer.
     *
     * @return index of the committed fill style, to be passed on to the
     *         import_xf::set_fill() method as its argument.
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

    /**
     * Commit the border style in the current buffer.
     *
     * @return index of the committed border style, to be passed on to the
     *         import_xf::set_border() method as its argument.
     */
    virtual size_t commit() = 0;
};

class ORCUS_DLLPUBLIC import_cell_protection
{
public:
    virtual ~import_cell_protection();

    /**
     * Hide the entire cell content when the sheet is protected.
     *
     * @param b whether to hide the entire cell content when the sheet is
     *          protected.
     */
    virtual void set_hidden(bool b) = 0;

    /**
     * Lock the cell when the sheet is protected.
     *
     * @param b whether or not to lock the cell when the sheet is protected.
     */
    virtual void set_locked(bool b) = 0;

    /**
     * Specify whether or not to print the cell content when the sheet is
     * protected.
     *
     *
     * @param b whether or not to print the cell content when the sheet is
     *          protected.
     */
    virtual void set_print_content(bool b) = 0;

    /**
     * Hide the formula when the sheet is protected and the cell contains
     * formula.
     *
     * @param b whether or not to hide the formula when the sheet is protected
     *          and the cell contains formula.
     */
    virtual void set_formula_hidden(bool b) = 0;

    /**
     * Commit the cell protection data in the current buffer.
     *
     * @return index of the committed cell protection data, to be passed on to
     *         the import_xf::set_protection() method as its argument.
     */
    virtual size_t commit() = 0;
};

class ORCUS_DLLPUBLIC import_number_format
{
public:
    virtual ~import_number_format();

    virtual void set_identifier(std::size_t id) = 0;
    virtual void set_code(std::string_view s) = 0;

    /**
     * Commit the number format data in the current buffer.
     *
     * @return index of the committed number format data, to be passed on to the
     *         import_xf::set_number_format() method as its argument.
     */
    virtual size_t commit() = 0;
};

/**
 * This interface is used to import cell format records for direct cell
 * formats, named cell style formats, and differential cell formats.
 *
 * The following cell format types:
 * <ul>
 *   <li>font</li>
 *   <li>fill</li>
 *   <li>border</li>
 *   <li>protection</li>
 *   <li>number format</li>
 * </ul>
 * use indices to reference their records in their respective record pools.
 *
 * The horizontal and vertical alignments are specified directly.
 */
class ORCUS_DLLPUBLIC import_xf
{
public:
    virtual ~import_xf();

    /**
     * Set the index of the font record, as returned from the
     * import_font_style::commit() method.
     *
     * @param index index of the font record to reference.
     */
    virtual void set_font(size_t index) = 0;

    /**
     * Set the index of the fill record, as returned from the
     * import_fill_style::commit() method.
     *
     * @param index index of the fill record to reference.
     */
    virtual void set_fill(size_t index) = 0;

    /**
     * Set the index of the border record, as returned from the
     * import_border_style::commit() method.
     *
     * @param index index of the border record to reference.
     */
    virtual void set_border(size_t index) = 0;

    /**
     * Set the index of the cell protection record, as returned from the
     * import_cell_protection::commit() method.
     *
     * @param index index of the cell protection record to reference.
     */
    virtual void set_protection(size_t index) = 0;

    /**
     * Set the index of the number format record, as returned from the
     * import_number_format::commit() method.
     *
     * @param index index of the number format record to reference.
     */
    virtual void set_number_format(size_t index) = 0;

    /**
     * Set the index into the cell style record to specify a named cell style it
     * uses as its base format in case the cell has an underlying style applied.
     * This is applicable only for a direct cell format i.e. when the xf
     * category is xf_category_t::cell.  If the category is
     * xf_category::cell_style, this value should not be used.
     *
     * @note This is so far Excel-specific.
     *
     * @param index index into the cell style record it uses as its basis.
     */
    virtual void set_style_xf(size_t index) = 0;

    virtual void set_apply_alignment(bool b) = 0;
    virtual void set_horizontal_alignment(hor_alignment_t align) = 0;
    virtual void set_vertical_alignment(ver_alignment_t align) = 0;

    /**
     * Specify whether or not to wrap text when the text spills over the cell
     * region.
     *
     * @param b whether or not to wrap text when the text spills over the cell
     *          region.
     */
    virtual void set_wrap_text(bool b) = 0;

    /**
     * Specify whether or not to shrink the text within cell until it fits
     * inside the cell.
     *
     * @param b whether or not to shrink the text.
     */
    virtual void set_shrink_to_fit(bool b) = 0;

    /**
     * Commit the cell format in the current buffer to the storage.
     *
     * @return index of the cell format data in the storage.  This index may be
     *         passed to the import_cell_style::set_xf() method.
     */
    virtual size_t commit() = 0;
};

/**
 * This interface is used to import named cell style records.
 *
 * @note The actual cell format data for named cell styles are imported
 *       through import_xf, and this interface references its index through
 *       the import_cell_style::set_xf() method.
 *
 */
class ORCUS_DLLPUBLIC import_cell_style
{
public:
    virtual ~import_cell_style();

    virtual void set_name(std::string_view s) = 0;

    /**
     * Set the index into the cell format record.  The named cell style applies
     * the format referenced by this index.
     *
     * @param index index into the cell format record.
     */
    virtual void set_xf(size_t index) = 0;

    /**
     * Set the index into the built-in cell style record.
     *
     * @note This is Excel-specific, and unclear whether it's useful outside of
     * Excel's implementation.  Built-in styles are not stored in file, and
     * Excel likely has its own internal styles stored in the application
     * itself.
     *
     * @param index index into the built-in cell style record.
     */
    virtual void set_builtin(size_t index) = 0;

    /**
     * Set the name of the parent cell style it uses as its basis.
     *
     * @note ODF uses this but Excel does not use this value.
     *
     * @param s name of the parent cell style.
     */
    virtual void set_parent_name(std::string_view s) = 0;

    virtual size_t commit() = 0;
};

}}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
