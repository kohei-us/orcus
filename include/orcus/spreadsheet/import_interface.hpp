/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef ORCUS_SPREADSHEET_IMPORT_INTERFACE_HPP
#define ORCUS_SPREADSHEET_IMPORT_INTERFACE_HPP

#include <cstdlib>

#include "types.hpp"
#include "../types.hpp"
#include "../env.hpp"

// NB: This header must not depend on ixion, as it needs to be usable for
// those clients that provide their own formula engine.  Other headers in
// the orcus::spreadsheet namespace may depend on ixion.

namespace orcus { namespace spreadsheet { namespace iface {

class import_pivot_cache_definition;
class import_pivot_cache_records;
class import_sheet_view;

/**
 * Interface class designed to be derived by the implementor.
 */
class import_shared_strings
{
public:
    ORCUS_DLLPUBLIC virtual ~import_shared_strings() = 0;

    /**
     * Append new string to the string list.  Order of insertion is important
     * since that determines the numerical ID values of inserted strings.
     * Note that this method assumes that the caller knows the string being
     * appended is not yet in the pool.
     *
     * @param s string to append to the pool.
     *
     * @return ID of the string just inserted.
     */
    virtual size_t append(std::string_view s) = 0;

    /**
     * Similar to the append method, it adds new string to the string pool;
     * however, this method checks if the string being added is already in the
     * pool before each insertion, to avoid duplicated strings.
     *
     * @param s string to add to the pool.
     *
     * @return ID of the string just inserted.
     */
    virtual size_t add(std::string_view s) = 0;

    /**
     * Set the index of a font to apply to the current format attributes.
     *
     * @param font_index positive integer representing the font to use.
     */
    virtual void set_segment_font(size_t font_index) = 0;

    /**
     * Set whether or not to make the font bold to the current format
     * attributes.
     *
     * @param b true if it's bold, false otherwise.
     */
    virtual void set_segment_bold(bool b) = 0;

    /**
     * Set whether or not to set the font italic font to the current format
     * attributes.
     *
     * @param b true if it's italic, false otherwise.
     */
    virtual void set_segment_italic(bool b) = 0;

    /**
     * Set the name of a font to the current format attributes.
     *
     * @param s font name.
     */
    virtual void set_segment_font_name(std::string_view s) = 0;

    /**
     * Set a font size to the current format attributes.
     *
     * @param point font size in points.
     */
    virtual void set_segment_font_size(double point) = 0;

    /**
     * Set the color of a font in ARGB to the current format attributes.
     *
     * @param alpha alpha component value (0-255).
     * @param red red component value (0-255).
     * @param green green component value (0-255).
     * @param blue blue component value (0-255).
     */
    virtual void set_segment_font_color(color_elem_t alpha, color_elem_t red, color_elem_t green, color_elem_t blue) = 0;

    /**
     * Append a string segment with the current format attributes to the
     * formatted string buffer.
     *
     * @param s string segment value.
     */
    virtual void append_segment(std::string_view s) = 0;

    /**
     * Store the formatted string in the current buffer to the shared strings
     * store.  The implementation may choose to unconditionally append the
     * string to the store, or choose to look for an existing indentical
     * formatted string to reuse and discard the new one if one exists.
     *
     * @return ID of the string just inserted, or the ID of an existing string
     *         with identical formatting attributes.
     */
    virtual size_t commit_segments() = 0;
};

/**
 * Interface for styles. Note that because the default style must have an
 * index of 0 in each style category, the caller must set the default styles
 * first before importing and setting real styles. ID's of styles are
 * assigned sequentially starting with 0 and upward in each style category.
 *
 * In contrast to xf formatting, dxf (differential formats) formatting only
 * stores the format information that is explicitly set. It does not store
 * formatting from the default style. Applying a dxf format to an object
 * only applies those explicitly set formats from the dxf entry, while all
 * the other formats are retained.
 */
class import_styles
{
public:
    ORCUS_DLLPUBLIC virtual ~import_styles() = 0;

    // font

    virtual void set_font_count(size_t n) = 0;
    virtual void set_font_bold(bool b) = 0;
    virtual void set_font_italic(bool b) = 0;
    virtual void set_font_name(std::string_view s) = 0;
    virtual void set_font_size(double point) = 0;
    virtual void set_font_underline(underline_t e) = 0;
    virtual void set_font_underline_width(underline_width_t e) = 0;
    virtual void set_font_underline_mode(underline_mode_t e) = 0;
    virtual void set_font_underline_type(underline_type_t e) = 0;
    virtual void set_font_underline_color(color_elem_t alpha, color_elem_t red, color_elem_t green, color_elem_t blue) = 0;
    virtual void set_font_color(color_elem_t alpha, color_elem_t red, color_elem_t green, color_elem_t blue) = 0;
    virtual void set_strikethrough_style(strikethrough_style_t s) = 0;
    virtual void set_strikethrough_type(strikethrough_type_t s) = 0;
    virtual void set_strikethrough_width(strikethrough_width_t s) = 0;
    virtual void set_strikethrough_text(strikethrough_text_t s) = 0;
    virtual size_t commit_font() = 0;

    // fill

    /**
     * Set the total number of fill styles.  This call is not strictly
     * required but may slightly improve performance.
     *
     * @param n number of fill styles.
     */
    virtual void set_fill_count(size_t n) = 0;

    /**
     * Set the type of fill pattern.
     *
     * @param fp fill pattern type.
     */
    virtual void set_fill_pattern_type(fill_pattern_t fp) = 0;

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
    virtual void set_fill_fg_color(color_elem_t alpha, color_elem_t red, color_elem_t green, color_elem_t blue) = 0;

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
    virtual void set_fill_bg_color(color_elem_t alpha, color_elem_t red, color_elem_t green, color_elem_t blue) = 0;

    /**
     * Commit the fill style currently in the buffer.
     *
     * @return the ID of the committed fill style, to be passed on to the
     *         set_xf_fill() method as its argument.
     */
    virtual size_t commit_fill() = 0;

    // border

    virtual void set_border_count(size_t n) = 0;

    virtual void set_border_style(border_direction_t dir, border_style_t style) = 0;
    virtual void set_border_color(
        border_direction_t dir, color_elem_t alpha, color_elem_t red, color_elem_t green, color_elem_t blue) = 0;
    virtual void set_border_width(border_direction_t dir, double width, orcus::length_unit_t unit) = 0;
    virtual size_t commit_border() = 0;

    // cell protection
    virtual void set_cell_hidden(bool b) = 0;
    virtual void set_cell_locked(bool b) = 0;
    virtual void set_cell_print_content(bool b) = 0;
    virtual void set_cell_formula_hidden(bool b) = 0;
    virtual size_t commit_cell_protection() = 0;

    // number format
    virtual void set_number_format_count(size_t n) = 0;
    virtual void set_number_format_identifier(size_t id) = 0;
    virtual void set_number_format_code(std::string_view s) = 0;
    virtual size_t commit_number_format() = 0;

    // cell format and cell style format (xf == cell format)

    virtual void set_cell_xf_count(size_t n) = 0;
    virtual void set_cell_style_xf_count(size_t n) = 0;
    virtual void set_dxf_count(size_t n) = 0;

    virtual void set_xf_font(size_t index) = 0;
    virtual void set_xf_fill(size_t index) = 0;
    virtual void set_xf_border(size_t index) = 0;
    virtual void set_xf_protection(size_t index) = 0;
    virtual void set_xf_number_format(size_t index) = 0;
    virtual void set_xf_style_xf(size_t index) = 0;
    virtual void set_xf_apply_alignment(bool b) = 0;
    virtual void set_xf_horizontal_alignment(hor_alignment_t align) = 0;
    virtual void set_xf_vertical_alignment(ver_alignment_t align) = 0;

    virtual size_t commit_cell_xf() = 0;
    virtual size_t commit_cell_style_xf() = 0;
    virtual size_t commit_dxf() = 0;

    // cell style entry

    virtual void set_cell_style_count(size_t n) = 0;
    virtual void set_cell_style_name(std::string_view s) = 0;
    virtual void set_cell_style_xf(size_t index) = 0;
    virtual void set_cell_style_builtin(size_t index) = 0;
    virtual void set_cell_style_parent_name(std::string_view s) = 0;
    virtual size_t commit_cell_style() = 0;
};

/**
 * Interface for importing sheet properties.  Sheet properties are those
 * that are used for decorative purposes but are not necessarily a part of
 * the sheet cell values.
 */
class import_sheet_properties
{
public:
    ORCUS_DLLPUBLIC virtual ~import_sheet_properties() = 0;

    virtual void set_column_width(col_t col, double width, orcus::length_unit_t unit) = 0;

    virtual void set_column_hidden(col_t col, bool hidden) = 0;

    virtual void set_row_height(row_t row, double height, orcus::length_unit_t unit) = 0;

    virtual void set_row_hidden(row_t row, bool hidden) = 0;

    /**
     * Specify merged cell range.
     *
     * @param range structure containing the top-left and bottom-right
     *              positions of the merged cell range.
     */
    virtual void set_merge_cell_range(const range_t& range) = 0;
};

/**
 * Interface for importing named expressions or ranges.
 *
 * Note that this interface has two different methods for defining named
 * expressions - set_named_expression() and set_named_range().
 *
 * The set_named_expression() method is generally used to pass named
 * expression strings.  The set_named_range() method is used only when the
 * format uses a different syntax to express a named range.  A named range
 * is a special case of named expression where the expression consists of
 * one range token.
 */
class ORCUS_DLLPUBLIC import_named_expression
{
public:
    virtual ~import_named_expression();

    /**
     * Specify an optional base position from which to evaluate a named
     * expression.  If not specified, the implementor should use the top-left
     * cell position on the first sheet as its implied base position.
     *
     * @param pos cell position to be used as the base.
     */
    virtual void set_base_position(const src_address_t& pos) = 0;

    /**
     * Define a new named expression or overwrite an existing one.
     *
     * @param name name of the expression to be defined.
     * @param expression expression to be associated with the name.
     */
    virtual void set_named_expression(std::string_view name, std::string_view expression) = 0;

    /**
     * Define a new named range or overwrite an existin gone.  Note that you
     * can only define one named range or expression per single commit.
     *
     * @param name name of the expression to be defined.
     * @param range range to be associated with the name.
     */
    virtual void set_named_range(std::string_view name, std::string_view range) = 0;

    virtual void commit() = 0;
};

/**
 * Interface for importing data tables.
 */
class import_data_table
{
public:
    ORCUS_DLLPUBLIC virtual ~import_data_table() = 0;

    virtual void set_type(data_table_type_t type) = 0;

    virtual void set_range(const range_t& range) = 0;

    virtual void set_first_reference(std::string_view ref, bool deleted) = 0;

    virtual void set_second_reference(std::string_view ref, bool deleted) = 0;

    virtual void commit() = 0;
};

class import_auto_filter
{
public:
    ORCUS_DLLPUBLIC virtual ~import_auto_filter() = 0;

    /**
     * Specify the range where the auto filter is applied.
     *
     * @param range structure containing the top-left and bottom-right
     *              positions of the auto filter range.
     */
    virtual void set_range(const range_t& range) = 0;

    /**
     * Specify the column position of a filter. The position is relative to
     * the first column in the auto filter range.
     *
     * @param col 0-based column position of a filter relative to the first
     *            column.
     */
    virtual void set_column(col_t col) = 0;

    /**
     * Add a match value to the current column filter.
     *
     * @param value match value.
     */
    virtual void append_column_match_value(std::string_view value) = 0;

    /**
     * Commit current column filter to the current auto filter.
     */
    virtual void commit_column() = 0;

    /**
     * Commit current auto filter to the model.
     */
    virtual void commit() = 0;
};

/**
 * This is an optional interface to import conditional formatting.
 *
 * A conditional format consists of:
 * <ul>
 *  <li>a range</li>
 *  <li>several entries</li>
 * </ul>
 *
 * Each entry consists of:
 * <ul>
 *   <li>a type</li>
 *   <li>a few properties depending on the type (optional)</li>
 *   <li>zero or more conditions depending on the type</li>
 * </ul>
 *
 * Each condition consists of:
 * <ul>
 *   <li>a formula/value/string</li>
 *   <li>a color (optional)</li>
 * </ul>
 */
class import_conditional_format
{
public:
    ORCUS_DLLPUBLIC virtual ~import_conditional_format() = 0;

    /**
     * Sets the color of the current condition.
     * only valid for type == databar or type == colorscale.
     */
    virtual void set_color(color_elem_t alpha, color_elem_t red,
            color_elem_t green, color_elem_t blue) = 0;

    /**
     * Sets the formula, value or string of the current condition.
     */
    virtual void set_formula(std::string_view formula) = 0;

    /**
     * Sets the type for the formula, value or string of the current condition.
     * Only valid for type = iconset, databar or colorscale.
     */
    virtual void set_condition_type(condition_type_t type) = 0;

    /**
     * Only valid for type = date.
     */
    virtual void set_date(condition_date_t date) = 0;

    /**
     * commits the current condition to the current entry.
     */
    virtual void commit_condition() = 0;

    /**
     * Name of the icons to use in the current entry.
     * only valid for type = iconset
     */
    virtual void set_icon_name(std::string_view name) = 0;

    /**
     * Use a gradient for the current entry.
     * only valid for type == databar
     */
    virtual void set_databar_gradient(bool gradient) = 0;

    /**
     * Position of the 0 axis in the current entry.
     * only valid for type == databar.
     */
    virtual void set_databar_axis(databar_axis_t axis) = 0;

    /**
     * Databar color for positive values.
     * only valid for type == databar.
     */
    virtual void set_databar_color_positive(color_elem_t alpha, color_elem_t red,
            color_elem_t green, color_elem_t blue) = 0;

    /**
     * Databar color for negative values.
     * only valid for type == databar.
     */
    virtual void set_databar_color_negative(color_elem_t alpha, color_elem_t red,
            color_elem_t green, color_elem_t blue) = 0;

    /**
     * Sets the minimum length for a databar.
     * only valid for type == databar.
     */
    virtual void set_min_databar_length(double length) = 0;

    /**
     * Sets the maximum length for a databar.
     * only valid for type == databar.
     */
    virtual void set_max_databar_length(double length) = 0;

    /**
     * Don't show the value in the cell.
     * only valid for type = databar, iconset, colorscale.
     */
    virtual void set_show_value(bool show) = 0;

    /**
     * Use the icons in reverse order.
     * only valid for type == iconset.
     */
    virtual void set_iconset_reverse(bool reverse) = 0;

    /**
     * TODO: In OOXML the style is stored as dxf and in ODF as named style.
     */
    virtual void set_xf_id(size_t xf) = 0;

    /**
     * Sets the current operation used for the current entry.
     * only valid for type == condition
     */
    virtual void set_operator(condition_operator_t condition_type) = 0;

    virtual void set_type(conditional_format_t type) = 0;

    virtual void commit_entry() = 0;

    virtual void set_range(std::string_view range) = 0;

    virtual void set_range(row_t row_start, col_t col_start,
            row_t row_end, col_t col_end) = 0;

    virtual void commit_format() = 0;
};

/**
 * Interface for table.  A table is a range within a sheet that consists of
 * one or more data columns with a header row that contains their labels.
 */
class ORCUS_DLLPUBLIC import_table
{
public:
    virtual ~import_table() = 0;

    virtual import_auto_filter* get_auto_filter();

    virtual void set_identifier(size_t id) = 0;

    virtual void set_range(std::string_view ref) = 0;

    virtual void set_totals_row_count(size_t row_count) = 0;

    virtual void set_name(std::string_view name) = 0;

    virtual void set_display_name(std::string_view name) = 0;

    virtual void set_column_count(size_t n) = 0;

    virtual void set_column_identifier(size_t id) = 0;
    virtual void set_column_name(std::string_view name) = 0;
    virtual void set_column_totals_row_label(std::string_view label) = 0;
    virtual void set_column_totals_row_function(totals_row_function_t func) = 0;
    virtual void commit_column() = 0;

    virtual void set_style_name(std::string_view name) = 0;
    virtual void set_style_show_first_column(bool b) = 0;
    virtual void set_style_show_last_column(bool b) = 0;
    virtual void set_style_show_row_stripes(bool b) = 0;
    virtual void set_style_show_column_stripes(bool b) = 0;

    virtual void commit() = 0;
};

class ORCUS_DLLPUBLIC import_formula
{
public:
    virtual ~import_formula();

    /**
     * Set the position of the cell.
     *
     * @param row row position.
     * @param col column position.
     */
    virtual void set_position(row_t row, col_t col) = 0;

    /**
     * Set formula string to the specified cell.
     *
     * @param grammar grammar to use to compile the formula string into
     *                tokens.
     * @param p pointer to the buffer where the formula string is stored.
     * @param n size of the buffer where the formula string is stored.
     */
    virtual void set_formula(formula_grammar_t grammar, const char* p, size_t n) = 0;

    /**
     * Register the formula as a shared string, to be shared with other cells.
     *
     * @param index shared string index to register the formula with.
     */
    virtual void set_shared_formula_index(size_t index) = 0;

    /**
     * Set cached result of string type.
     *
     * @param p pointer to the buffer where the string result is stored.
     * @param n size of the buffer where the string result is stored.
     */
    virtual void set_result_string(const char* p, size_t n) = 0;

    /**
     * Set cached result of numeric type.
     *
     * @param value numeric value to set as a cached result.
     */
    virtual void set_result_value(double value) = 0;

    /**
     * Set cached result of boolean type.
     *
     * @param value boolean value to set as a cached result.
     */
    virtual void set_result_bool(bool value) = 0;

    /**
     * Set empty value as a cached result.
     */
    virtual void set_result_empty() = 0;

    /**
     * Commit all the formula data to the specified cell.
     */
    virtual void commit() = 0;
};

class ORCUS_DLLPUBLIC import_array_formula
{
public:
    virtual ~import_array_formula();

    virtual void set_range(const range_t& range) = 0;

    virtual void set_formula(formula_grammar_t grammar, const char* p, size_t n) = 0;

    virtual void set_result_string(row_t row, col_t col, const char* p, size_t n) = 0;

    virtual void set_result_value(row_t row, col_t col, double value) = 0;

    virtual void set_result_bool(row_t row, col_t col, bool value) = 0;

    virtual void set_result_empty(row_t row, col_t col) = 0;

    virtual void commit() = 0;
};

/**
 * Interface for sheet.
 */
class ORCUS_DLLPUBLIC import_sheet
{
public:
    virtual ~import_sheet() = 0;

    virtual import_sheet_view* get_sheet_view();

    virtual import_sheet_properties* get_sheet_properties();

    /**
     * Get an interface for importing data tables.  Note that the implementer
     * may decide not to support this feature in which case this method
     * returns NULL.  The implementer is responsible for managing the life
     * cycle of the returned interface object.
     *
     * The implementor should also initialize the internal state of the
     * temporary data table object when this method is called.
     *
     * @return pointer to the data table interface object.
     */
    virtual import_data_table* get_data_table();

    /**
     * Get an interface for importing auto filter ranges.
     *
     * The implementor should also initialize the internal state of the
     * temporary auto filter object when this method is called.
     *
     * @return pointer to the auto filter interface object.
     */
    virtual import_auto_filter* get_auto_filter();

    /**
     * Get an interface for importing tables.  The implementer is responsible
     * for managing the life cycle of the returned interface object.
     *
     * The implementor should also initialize the internal state of the
     * temporary table object when this method is called.
     *
     * @return pointer to the table interface object, or NULL if the
     *         implementer doesn't support importing of tables.
     */
    virtual import_table* get_table();

    /**
     * get an interface for importing conditional formats. The implementer is responsible
     * for managing the life cycle of the returned interface object.
     *
     * @return pointer to the conditional format interface object, or NULL
     *          if the implementer doesn't support importing conditional formats.
     */
    virtual import_conditional_format* get_conditional_format();

    virtual import_named_expression* get_named_expression();

    virtual import_array_formula* get_array_formula();

    /**
     * Get an interface for importing formula cells.
     *
     * @return pointer to the formula interface object, or nullptr if the
     *         implementer doesn't support importing of formula cells.
     */
    virtual import_formula* get_formula();

    /**
     * Set raw string value to a cell and have the implementation
     * auto-recognize its data type.
     *
     * @param row row ID
     * @param col column ID
     * @param s raw string value.
     */
    virtual void set_auto(row_t row, col_t col, std::string_view s) = 0;

    /**
     * Set string value to a cell.
     *
     * @param row row ID
     * @param col column ID
     * @param sindex 0-based string index in the shared string table.
     */
    virtual void set_string(row_t row, col_t col, string_id_t sindex) = 0;

    /**
     * Set numerical value to a cell.
     *
     * @param row row ID
     * @param col column ID
     * @param value value being assigned to the cell.
     */
    virtual void set_value(row_t row, col_t col, double value) = 0;

    /**
     * Set a boolean value to a cell.
     *
     * @param row row ID
     * @param col col ID
     * @param value boolean value being assigned to the cell
     */
    virtual void set_bool(row_t row, col_t col, bool value) = 0;

    /**
     * Set date and time value to a cell.
     *
     * @param row row ID
     * @param col column ID
     * @param year 1-based value representing year
     * @param month 1-based value representing month, varying from 1 through
     *              12.
     * @param day 1-based value representing day, varying from 1 through 31.
     * @param hour the hour of a day, ranging from 0 through 23.
     * @param minute the minute of an hour, ranging from 0 through 59.
     * @param second the second of a minute, ranging from 0 through 59.
     */
    virtual void set_date_time(
        row_t row, col_t col,
        int year, int month, int day, int hour, int minute, double second) = 0;

    /**
     * Set cell format to specified cell.  The cell format is referred to by
     * the xf (cell format) index in the styles table.
     *
     * @param row row ID
     * @param col column ID
     * @param xf_index 0-based xf (cell format) index
     */
    virtual void set_format(row_t row, col_t col, size_t xf_index) = 0;

    /**
     * Set cell format to specified cell range.  The cell format is referred
     * to by the xf (cell format) index in the styles table.
     *
     * @param row_start start row ID
     * @param col_start start column ID
     * @param row_end end row ID
     * @param col_end end column ID
     * @param xf_index 0-based xf (cell format) index
     */
    virtual void set_format(row_t row_start, col_t col_start,
        row_t row_end, col_t col_end, size_t xf_index) = 0;

    /**
     * Duplicate the value of the source cell to one or more cells located
     * immediately below it.
     *
     * @param src_row row ID of the source cell
     * @param src_col column ID of the source cell
     * @param range_size number of cells below the source cell to copy the
     *                   source cell value to.  It must be at least one.
     */
    virtual void fill_down_cells(row_t src_row, col_t src_col, row_t range_size) = 0;

    /**
     * Get the size of the sheet.
     *
     * @return structure containing the numbers of rows and columns of the
     *         sheet.
     */
    virtual range_size_t get_sheet_size() const = 0;
};

class import_global_settings
{
public:
    ORCUS_DLLPUBLIC virtual ~import_global_settings() = 0;

    /**
     * Set the date that is to be represented by a value of 0.  All date
     * values will be internally represented relative to this date afterward.
     *
     * @param year 1-based value representing year
     * @param month 1-based value representing month, varying from 1 through
     *              12.
     * @param day 1-based value representing day, varying from 1 through 31.
     */
    virtual void set_origin_date(int year, int month, int day) = 0;

    /**
     * Set formula grammar to be used globally when parsing formulas if the
     * grammar is not specified.  This grammar will also be used when parsing
     * range strings associated with shared formula ranges, array formula
     * ranges, autofilter ranges etc.
     *
     * @param grammar default formula grammar
     */
    virtual void set_default_formula_grammar(formula_grammar_t grammar) = 0;

    /**
     * Get current default formula grammar.
     *
     * @return current default formula grammar.
     */
    virtual formula_grammar_t get_default_formula_grammar() const = 0;

    /**
     * Set the character set to be used when parsing string values.
     *
     * @param charset character set to apply when parsing string values.
     */
    virtual void set_character_set(character_set_t charset) = 0;
};

class ORCUS_DLLPUBLIC import_reference_resolver
{
public:
    virtual ~import_reference_resolver();

    /**
     * Resolve a textural representation of a single cell address.
     *
     * @param p pointer to the first character of the single cell address
     *          string.
     * @param n size of the single cell address string.
     *
     * @return structure containing the column and row positions of the
     *         address.
     *
     * @exception orcus::invalid_arg_error the string is not a valid
     *        single cell addreess.
     */
    virtual src_address_t resolve_address(const char* p, size_t n) = 0;

    /**
     * Resolve a textural representation of a range address.  Note that a
     * string representing a valid single cell address should be considered a
     * valid range address.
     *
     * @param p pointer to the first character of the range address string.
     * @param n size of the range address string.
     *
     * @return structure containing the start and end positions of the range
     *         address.
     *
     * @exception invalid_arg_error the string is not a valid range addreess.
     */
    virtual src_range_t resolve_range(const char* p, size_t n) = 0;
};

/**
 * This interface provides the filters a means to instantiate concrete
 * classes that implement the above interfaces.  The client code never has
 * to manually delete objects returned by its methods; the implementor of
 * this interface must manage the life cycles of objects it returns.
 *
 * The implementor of this interface normally wraps the document instance
 * inside it and have the document instance manage the life cycles of
 * various objects it creates.
 */
class ORCUS_DLLPUBLIC import_factory
{
public:
    virtual ~import_factory() = 0;

    virtual import_global_settings* get_global_settings();

    /**
     * @return pointer to the shared strings instance. It may return NULL if
     *         the client app doesn't support shared strings.
     */
    virtual import_shared_strings* get_shared_strings();

    virtual import_named_expression* get_named_expression();

    /**
     * @return pointer to the styles instance. It may return NULL if the
     *         client app doesn't support styles.
     */
    virtual import_styles* get_styles();

    virtual import_reference_resolver* get_reference_resolver(formula_ref_context_t cxt);

    /**
     * Create an interface for pivot cache definition import for a specified
     * cache ID.  In case a pivot cache alrady exists for the passed ID, the
     * client app should overwrite the existing cache with a brand-new cache
     * instance.
     *
     * @param cache_id numeric ID associated with the pivot cache.
     *
     * @return pointer to the pivot cache interface instance. If may return
     *         NULL if the client app doesn't support pivot tables.
     */
    virtual import_pivot_cache_definition* create_pivot_cache_definition(
        pivot_cache_id_t cache_id);

    /**
     * Create an interface for pivot cache records import for a specified
     * cache ID.
     *
     * @param cache_id numeric ID associated with the pivot cache.
     *
     * @return pointer to the pivot cache records interface instance.  If may
     *         return nullptr if the client app doesn't support pivot tables.
     */
    virtual import_pivot_cache_records* create_pivot_cache_records(
        pivot_cache_id_t cache_id);

    /**
     * Append a sheet with specified sheet position index and name.
     *
     * @param sheet_index position index of the sheet to be appended.  It is
     *                    0-based i.e. the first sheet to be appended will
     *                    have an index value of 0.
     * @param sheet_name pointer to the first character in the buffer where
     *                   the sheet name is stored.
     * @param sheet_name_length length of the sheet name.
     *
     * @return pointer to the sheet instance. It may return nullptr if the
     *         client app fails to append a new sheet.
     */
    virtual import_sheet* append_sheet(
        sheet_t sheet_index, const char* sheet_name, size_t sheet_name_length) = 0;

    /**
     * @return pointer to the sheet instance whose name matches the name
     *         passed to this method. It returns nullptr if no sheet instance
     *         exists by the specified name.
     */
    virtual import_sheet* get_sheet(const char* sheet_name, size_t sheet_name_length) = 0;

    /**
     * Retrieve sheet instance by specified numerical sheet index.
     *
     * @param sheet_index sheet index
     *
     * @return pointer to the sheet instance, or nullptr if no sheet instance
     *         exists at specified sheet index position.
     */
    virtual import_sheet* get_sheet(sheet_t sheet_index) = 0;

    /**
     * This method is called at the end of import, to give the implementor a
     * chance to perform post-processing if necessary.
     */
    virtual void finalize() = 0;
};

}}}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
