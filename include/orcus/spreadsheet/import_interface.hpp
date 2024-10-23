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

class import_auto_filter;
class import_styles;
class import_pivot_cache_definition;
class import_pivot_cache_records;
class import_sheet_view;
class import_strikethrough;
class import_underline;

/**
 * Interface for importing raw string values shared in string cells.  String
 * values may be either with or without formatted segments.
 *
 * To insert an unformatted string, simply use either append() or add()
 * method.  The string will then be immediately pushed to the pool.
 *
 * To insert a string with mixed formatted segments, you need to first use one
 * or more of:
 *
 * @li set_segment_font()
 * @li set_segment_bold()
 * @li set_segment_italic()
 * @li set_segment_font_name()
 * @li set_segment_font_size()
 * @li set_segment_font_color()
 *
 * to define the format attribute(s) of a string segment followed by a call to
 * append_segment().  This may be repeated as many times as necessary.  Then
 * as the final step, call commit_segments() to insert the entire series of
 * formatted segments to the pool as a single string entry.  The following
 * example demonstrates how the code may look like:
 *
 * @code{.cpp}
 * import_shared_strings* iface = ...;
 *
 * // store a segment with specific font, size and boldness.
 * iface->set_segment_font_name("FreeMono");
 * iface->set_segment_font_size(14);
 * iface->set_segment_font_bold(true);
 * iface->append_segment("a bold and big segment");
 *
 * // store an unformatted segment.
 * iface->append_segment(" followed by ");
 *
 * // store a segment with smaller, italic font.
 * iface->set_segment_font_size(7);
 * iface->set_segment_font_italic(true);
 * iface->append_segment("a small and italic segment");
 *
 * iface->commit_segments(); // commit the whole formatted string to the pool.
 * @endcode
 */
class ORCUS_DLLPUBLIC import_shared_strings
{
public:
    virtual ~import_shared_strings();

    /**
     * Append a new string to the sequence of strings.  Order of insertion
     * determines the numerical ID value of an inserted string. Note that this
     * method assumes that the caller knows the string being appended is not yet
     * in the pool; it does not check on duplicated strings.
     *
     * @param s string to append to the pool.
     *
     * @return ID of the inserted string.
     */
    virtual size_t append(std::string_view s) = 0;

    /**
     * Similar to the append() method, it adds a new string to the string pool;
     * however, this method checks if the string being added is already in the
     * pool before each insertion, to avoid duplicated strings.
     *
     * @param s string to add to the pool.
     *
     * @return ID of the inserted string.
     */
    virtual size_t add(std::string_view s) = 0;

    /**
     * Set the index of a font to apply to the current format attributes.  Refer
     * to the import_font_style interface on how to obtain a font index.  Note
     * that a single font index is associated with multiple font-related
     * formatting attributes, such as font name, font color, boldness and
     * italics.
     *
     * @param font_index positive integer representing the font to use.
     */
    virtual void set_segment_font(size_t font_index) = 0;

    /**
     * Set whether or not to make the current segment bold.
     *
     * @param b true if it's bold, false otherwise.
     */
    virtual void set_segment_bold(bool b) = 0;

    /**
     * Set whether or not to make the current segment italic.
     *
     * @param b true if it's italic, false otherwise.
     */
    virtual void set_segment_italic(bool b) = 0;

    /**
     * Set whether or not to apply superscript to the current segment.
     *
     * @param b True if superscript needs to be applied, false otherwise.
     */
    virtual void set_segment_superscript(bool b) = 0;

    /**
     * Set whether or not to apply subscript to the current segment.
     *
     * @param b True if subscript needs to be applied, false otherwise.
     */
    virtual void set_segment_subscript(bool b) = 0;

    /**
     * Set the name of a font to the current segment.
     *
     * @param s font name.
     */
    virtual void set_segment_font_name(std::string_view s) = 0;

    /**
     * Set a font size to the current segment.
     *
     * @param point font size in points.
     */
    virtual void set_segment_font_size(double point) = 0;

    /**
     * Set the color of a font in ARGB format to the current segment.
     *
     * @param alpha alpha component value (0-255).
     * @param red red component value (0-255).
     * @param green green component value (0-255).
     * @param blue blue component value (0-255).
     */
    virtual void set_segment_font_color(color_elem_t alpha, color_elem_t red, color_elem_t green, color_elem_t blue) = 0;

    /**
     * Get an interface for importing the underline attributes and applying them
     * to the current segment.
     *
     * @return Pointer to an interface for applying the underline-related
     *         attributes to the current segment.  The implementer may return
     *         @p nullptr if the implementation does not support it.
     */
    virtual import_underline* start_underline();

    /**
     * Get an interface for importing the strikethrough attributes and applying
     * them to the current segment.
     *
     * @return Pointer to an interface for applying the strikethrough-related
     *         attributes to the current segment.  The implementer may return
     *         @p nullptr if the implementation does not support it.
     */
    virtual import_strikethrough* start_strikethrough();

    /**
     * Push the current string segment to the buffer. Any formatting attributes
     * set so far will be applied to this segment.
     *
     * @param s string value for the segment.
     */
    virtual void append_segment(std::string_view s) = 0;

    /**
     * Store the entire formatted string in the current buffer to the shared
     * strings pool.  The implementor may choose to unconditionally append the
     * string to the pool, or choose to find an existing duplicate and reuse
     * it instead.
     *
     * @return ID of the string just inserted, or the ID of an existing string
     *         with identical formatting.
     */
    virtual size_t commit_segments() = 0;
};

/**
 * Interface for importing sheet properties.  Sheet properties include:
 *
 * @li column widths and row heights,
 * @li hidden flags for columns and rows, and
 * @li merged cell ranges.
 *
 * These properties are independent of the cell contents of a sheet.
 */
class ORCUS_DLLPUBLIC import_sheet_properties
{
public:
    virtual ~import_sheet_properties();

    /**
     * Set a column width to one or more columns.
     *
     * @param col      0-based position of the first column.
     * @param col_span number of contiguous columns to apply the width to.
     * @param width    column width to apply.
     * @param unit     unit of measurement to use for the width value.
     */
    virtual void set_column_width(col_t col, col_t col_span, double width, orcus::length_unit_t unit) = 0;

    /**
     * Set a column hidden flag to one or more columns.
     *
     * @param col      0-based position of the first column.
     * @param col_span number of contiguous columns to apply the flag to.
     * @param hidden   flag indicating whether or not the columns are hidden.
     */
    virtual void set_column_hidden(col_t col, col_t col_span, bool hidden) = 0;

    /**
     * Set a row height to specified row.
     *
     * @param row 0-based position of a row.
     * @param row_span number of contiguous rows to apply the height to.
     * @param height new row height value to set.
     * @param unit unit of the new row height value.
     *
     * @todo Convert this to take a raw span.
     */
    virtual void set_row_height(row_t row, row_t row_span, double height, orcus::length_unit_t unit) = 0;

    /**
     * Set a row hidden flag to a specified row.
     *
     * @param row    0-based position of a row.
     * @param row_span number of contiguous rows to apply the flag to.
     * @param hidden flag indicating whether or not the row is hidden.
     *
     * @todo Convert this to take a raw span.
     */
    virtual void set_row_hidden(row_t row, row_t row_span, bool hidden) = 0;

    /**
     * Set a merged cell range.
     *
     * @param range structure containing the top-left and bottom-right
     *              positions of a merged cell range.
     */
    virtual void set_merge_cell_range(const range_t& range) = 0;
};

/**
 * Interface for importing named expressions or ranges.
 *
 * This interface has two different methods for defining named expressions:
 *
 * @li set_named_expression() and
 * @li set_named_range().
 *
 * Generally speaking, set_named_expression() can be used to define both named
 * expression and named range.  However, the implementor may choose to apply a
 * different syntax rule to parse an expression passed to set_named_range(),
 * depending on the formula grammar defined via @ref
 * import_global_settings::set_default_formula_grammar().  For instance, the
 * OpenDocument Spreadsheet format is known to use different syntax rules
 * between named expressions and named ranges.
 *
 * A named range is a special case of a named expression where the expression
 * consists of only one single cell range token.
 *
 * Here is a code example of how a named expression is defined:
 *
 * @code{.cpp}
 * import_named_expression* iface = ...;
 *
 * // set the A1 on the first sheet as its origin (optional).
 * src_address_t origin{0, 0, 0};
 * iface->set_base_position(origin);
 * iface->set_named_expression("MyExpression", "SUM(A1:B10)+SUM(D1:D4)");
 * iface->commit();
 * @endcode
 *
 * Replace the above set_named_expression() call with set_named_range() if you
 * wish to define a named range instead.
 */
class ORCUS_DLLPUBLIC import_named_expression
{
public:
    virtual ~import_named_expression();

    /**
     * Specify an optional base position, or origin, from which to evaluate a
     * named expression.  If not specified, the implementor should use the
     * top-left corner cell on the first sheet as its origin.
     *
     * @param pos cell position to be used as the origin.
     */
    virtual void set_base_position(const src_address_t& pos) = 0;

    /**
     * Set a named expression to the buffer.
     *
     * @param name name of the expression to be defined.
     * @param expression expression to be associated with the name.
     */
    virtual void set_named_expression(std::string_view name, std::string_view expression) = 0;

    /**
     * Set a named range to the buffer.
     *
     * @param name name of the expression to be defined.
     * @param range range to be associated with the name.
     */
    virtual void set_named_range(std::string_view name, std::string_view range) = 0;

    /**
     * Commit the named expression or range currently in the buffer to the
     * document.
     */
    virtual void commit() = 0;
};

/**
 * Interface for importing data tables.
 */
class ORCUS_DLLPUBLIC import_data_table
{
public:
    virtual ~import_data_table();

    /**
     * Set the type of a data table.  A data table can either:
     *
     * @li be a single-variable column-oriented,
     * @li be a single-variable row-oriented, or
     * @li use two variables that use both column and row.
     *
     * @param type type of a data table.
     */
    virtual void set_type(data_table_type_t type) = 0;

    /**
     * Set the range of a data table.
     *
     * @param range range of a data table.
     */
    virtual void set_range(const range_t& range) = 0;

    /**
     * Set the reference of the first input cell.
     *
     * @param ref     reference of the first input cell.
     * @param deleted whether or not this input cell has been deleted.
     */
    virtual void set_first_reference(std::string_view ref, bool deleted) = 0;

    /**
     * Set the reference of the second input cell but only if the data table
     * uses two variables.
     *
     * @note This method gets called only if the data table uses two variables.
     *
     * @param ref     reference of the second input cell.
     * @param deleted whether or not this input cell has been deleted.
     */
    virtual void set_second_reference(std::string_view ref, bool deleted) = 0;

    /**
     * Store the current data table data in the buffer to the backend sheet
     * storage.
     */
    virtual void commit() = 0;
};

/**
 * This is an optional interface to import conditional formatting.
 *
 * In general, a single conditional format consists of:
 *
 * @li a cell range the format is applied to, and
 * @li one or more rule entries.
 *
 * Each rule entry consists of:
 *
 * @li a type of rule,
 * @li zero or more rule properties, and
 * @li zero or more conditions depending on the rule type.
 *
 * Lastly, each condition consists of:
 *
 * @li a formula, value, or string,
 * @li an optional color.
 *
 * The flow of the import process varies depending on the type of the
 * conditional formatting being imported.  The following is an example of
 * importing a conditional formatting that consists of a rule that applies a
 * format when the cell value is greather than 2:
 *
 * @code{.cpp}
 * import_conditional_format* iface = ... ;
 *
 * iface->set_range("A2:A13");
 * iface->set_xf_id(14); // apply differential format (dxf) whose ID is 14
 * iface->set_type(conditional_format_t::condition); // rule entry type
 * iface->set_operator(condition_operator_t::expression);
 * iface->set_operator(condition_operator_t::greater);
 *
 * iface->set_formula("2");
 * iface->commit_condition();
 *
 * iface->commit_entry();
 *
 * iface->commit_format();
 * @endcode
 *
 * @todo Revise this API for simplification.
 */
class ORCUS_DLLPUBLIC import_conditional_format
{
public:
    virtual ~import_conditional_format();

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
 * Interface for table.  A table is a range of cells within a sheet that
 * consists of one or more data columns with a header row that contains their
 * labels.
 */
class ORCUS_DLLPUBLIC import_table
{
public:
    virtual ~import_table();

    /**
     * Get an optional interface for importing auto filter data stored as part
     * of a table.
     *
     * The implementor should initialize the internal buffer to store the auto
     * filter data when this method is called.
     *
     * @param range Filtered range.
     *
     * @return Pointer to an auto filter interface object, or a @p nullptr if
     *         the implementor doesn't support it.
     */
    virtual import_auto_filter* start_auto_filter(const range_t& range);

    /**
     * Set an integral identifier unique to the table.
     *
     * @param id identifier associated with the table.
     */
    virtual void set_identifier(size_t id) = 0;

    /**
     * Set a 2-dimensional cell range associated with the table.
     *
     * @param range cell range associated with the table.
     */
    virtual void set_range(const range_t& range) = 0;

    /**
     * Set the number of totals rows.
     *
     * @param row_count number of totals rows.
     */
    virtual void set_totals_row_count(size_t row_count) = 0;

    /**
     * Set the internal name of the table.
     *
     * @param name name of the table.
     */
    virtual void set_name(std::string_view name) = 0;

    /**
     * Set the displayed name of the table.
     *
     * @param name displayed name of the table.
     */
    virtual void set_display_name(std::string_view name) = 0;

    /**
     * Set the number of columns the table contains.
     *
     * @param n number of columns in the table.
     *
     * @note This method gets called before the column data gets imported.  The
     *       implementor can use this call to initialize the buffer for storing
     *       the column data.
     */
    virtual void set_column_count(size_t n) = 0;

    /**
     * Set an integral identifier for a column.
     *
     * @param id integral identifier for a column.
     */
    virtual void set_column_identifier(size_t id) = 0;

    /**
     * Set a name of a column.
     *
     * @param name name of a column.
     */
    virtual void set_column_name(std::string_view name) = 0;

    /**
     * Set the totals row label for a column.
     *
     * @param label row label for a column.
     */
    virtual void set_column_totals_row_label(std::string_view label) = 0;

    /**
     * Set the totals row function for a column.
     *
     * @param func totals row function for a column.
     */
    virtual void set_column_totals_row_function(totals_row_function_t func) = 0;

    /**
     * Push and append the column data stored in the current column data buffer
     * into the table buffer.
     */
    virtual void commit_column() = 0;

    /**
     * Set the name of a style to apply to the table.
     *
     * @param name name of a style to apply to the table.
     */
    virtual void set_style_name(std::string_view name) = 0;

    /**
     * Specify whether or not the first column in the table should have the
     * style applied.
     *
     * @param b whether or not the first column in the table should have the
     *          style applied.
     */
    virtual void set_style_show_first_column(bool b) = 0;

    /**
     * Specify whether or not the last column in the table should have the style
     * applied.
     *
     * @param b whether or not the last column in the table should have the
     *          style applied.
     */
    virtual void set_style_show_last_column(bool b) = 0;

    /**
     * Specify whether or not row stripe formatting is applied.
     *
     * @param b whether or not row stripe formatting is applied.
     */
    virtual void set_style_show_row_stripes(bool b) = 0;

    /**
     * Specify whether or not column stripe formatting is applied.
     *
     * @param b whether or not column stripe formatting is applied.
     */
    virtual void set_style_show_column_stripes(bool b) = 0;

    /**
     * Push the data stored in the table buffer into the document store.
     */
    virtual void commit() = 0;
};

/**
 * Interface for importing the properties of a single formula cell.  A formula
 * cell contains a formula expression that can be computed, and optionally a
 * cached result of the last computation performed on the expression.
 */
class ORCUS_DLLPUBLIC import_formula
{
public:
    virtual ~import_formula();

    /**
     * Set the position of a cell.
     *
     * @param row row position.
     * @param col column position.
     */
    virtual void set_position(row_t row, col_t col) = 0;

    /**
     * Set formula string to a cell.
     *
     * @param grammar grammar to use to compile the formula string into
     *                tokens.
     * @param formula formula expression to store.
     */
    virtual void set_formula(formula_grammar_t grammar, std::string_view formula) = 0;

    /**
     * Register the formula stored in a cell as a shared formula to be shared
     * with other cells, if the cell contains a formula string.
     *
     * If a cell references a shared formula stored in another cell, only
     * specify the index of that shared formula without specifying a formula
     * string of its own.  In that case, it is expected that another formula
     * cell registers its formula string for that index.
     *
     * @param index shared string index to register the formula with.
     */
    virtual void set_shared_formula_index(size_t index) = 0;

    /**
     * Set cached result of string type.
     *
     * @param value string result value.
     */
    virtual void set_result_string(std::string_view value) = 0;

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

/**
 * Interface for importing the properties of an array formula which occupies a
 * range of cells.  Cells that are part of an array formula share the same
 * formula expression but may have different calculation results.
 */
class ORCUS_DLLPUBLIC import_array_formula
{
public:
    virtual ~import_array_formula();

    /**
     * Set the range of an array formula.
     *
     * @param range range of an array formula.
     */
    virtual void set_range(const range_t& range) = 0;

    /**
     * Set the formula expression of an array formula.
     *
     * @param grammar grammar to use to compile the formula string into
     *                tokens.
     * @param formula formula expression of an array formula.
     */
    virtual void set_formula(formula_grammar_t grammar, std::string_view formula) = 0;

    /**
     * Set a cached string result of a cell within the array formula range.
     *
     * @param row 0-based row position of a cell.
     * @param col 0-based column position of a cell.
     * @param value cached string value to set.
     */
    virtual void set_result_string(row_t row, col_t col, std::string_view value) = 0;

    /**
     * Set a cached numeric result of a cell within the array formula range.
     *
     * @param row 0-based row position of a cell.
     * @param col 0-based column position of a cell.
     * @param value cached numeric value to set.
     */
    virtual void set_result_value(row_t row, col_t col, double value) = 0;

    /**
     * Set a cached boolean result of a cell within the array formula range.
     *
     * @param row 0-based row position of a cell.
     * @param col 0-based column position of a cell.
     * @param value cached boolean value to set.
     */
    virtual void set_result_bool(row_t row, col_t col, bool value) = 0;

    /**
     * Set an empty value as a cached result to a cell within the array formula
     * range.
     *
     * @param row 0-based row position of a cell.
     * @param col 0-based column position of a cell.
     */
    virtual void set_result_empty(row_t row, col_t col) = 0;

    /**
     * Push the properties of an array formula currently stored in the buffer to
     * the sheet store.
     */
    virtual void commit() = 0;
};

/**
 * Interface for importing the content and properties of a sheet.
 */
class ORCUS_DLLPUBLIC import_sheet
{
public:
    virtual ~import_sheet();

    /**
     * Get an optional interface for importing properties that are specific to a
     * view of a sheet.
     *
     * @return pointer to the interface for importing view properties, or a @p
     *         nullptr if the implementor doesn't support it.
     */
    virtual import_sheet_view* get_sheet_view();

    /**
     * Get an optional interface for importing sheet properties.
     *
     * @return pointer to the interface for importing sheet properties, or a @p
     *         nullptr if the implementor doesn't support it.
     */
    virtual import_sheet_properties* get_sheet_properties();

    /**
     * Get an optional interface for importing data tables.  Note that the
     * implementer may decide not to support this feature in which case this
     * method should return a @p nullptr.
     *
     * The implementor should initialize the internal state of the temporary
     * data table object when this method is called.
     *
     * @return pointer to the data table interface object, or a @p nullptr if
     *         the implementor doesn't support it.
     */
    virtual import_data_table* get_data_table();

    /**
     * Get an optional interface for importing auto filter data.
     *
     * The implementor should initialize the internal buffer to store the auto
     * filter data when this method is called.
     *
     * @param range Filtered range.
     *
     * @return Pointer to an auto filter interface object, or a @p nullptr if
     *         the implementor doesn't support it.
     */
    virtual import_auto_filter* start_auto_filter(const range_t& range);

    /**
     * Get an interface for importing tables.
     *
     * The implementor should initialize the internal state of the temporary
     * table object when this method is called.
     *
     * @return pointer to a table interface object, or @p nullptr if the
     *         implementer doesn't support importing of tables.
     */
    virtual import_table* start_table();

    /**
     * Get an optional interface for importing conditional formats.
     *
     * @return pointer to the conditional format interface object, or @p nullptr
     *          if the implementer doesn't support importing conditional
     *          formats.
     */
    virtual import_conditional_format* get_conditional_format();

    /**
     * Get an optional interface for importing sheet-local named expressions.
     *
     * @return pointer to the sheet-local named expression interface, or a @p
     *         nullptr if the implementor doesn't support it.
     */
    virtual import_named_expression* get_named_expression();

    /**
     * Get an optional interface for importing array formulas.  An array formula
     * is a formula expression applied to a range of cells where each cell may
     * have a different result value.
     *
     * @return pointer to the array formula import interface, or a @p nullptr if
     *         the implementor doesn't support it.
     */
    virtual import_array_formula* get_array_formula();

    /**
     * Get an optional interface for importing formula cells.
     *
     * @return pointer to the formula interface object, or a @p nullptr if the
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
     * @note This method gets called after both set_column_format() and
     *       set_row_format().
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
     * Set cell format to a specified column.  The cell format is referred to by
     * the xf (cell format) index in the styles table.
     *
     * @note This method gets called first before set_row_format() or
     *       set_format() variants.
     *
     * @param col column ID
     * @param col_span number of contiguous columns to apply the format to. It
     *                 must be at least one.
     * @param xf_index 0-based xf (cell format) index
     */
    virtual void set_column_format(col_t col, col_t col_span, std::size_t xf_index) = 0;

    /**
     * Set cell format to a specified row.  The cell format is referred to by
     * the xf (cell format) index in the styles table.
     *
     * @note This method gets called after set_column_format() but before
     *       set_format().
     *
     * @param row row ID
     * @param xf_index 0-based xf (cell format) index
     */
    virtual void set_row_format(row_t row, std::size_t xf_index) = 0;

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

/**
 * Interface for specifying global settings that may affect how the
 * implementor should process certain values and properties.
 */
class ORCUS_DLLPUBLIC import_global_settings
{
public:
    virtual ~import_global_settings();

    /**
     * Set the date that is to be represented by a value of 0.  All date
     * values should be represented relative to this date.  This may affect, for
     * instance, values imported via @ref import_sheet::set_date_time().
     *
     * @param year 1-based value representing year
     * @param month 1-based value representing month, varying from 1 through
     *              12.
     * @param day 1-based value representing day, varying from 1 through 31.
     */
    virtual void set_origin_date(int year, int month, int day) = 0;

    /**
     * Set the formula grammar to be used globally when parsing formulas if the
     * grammar is not specified.  This grammar should also be used when parsing
     * range strings associated with shared formula ranges, array formula
     * ranges, autofilter ranges etc.
     *
     * Note that the import filter may specify what formula grammar to use
     * locally when importing formula expressions for cells via @ref
     * import_formula::set_formula(), in which case the implementor should honor
     * that one instead.
     *
     * @param grammar default formula grammar to use globally unless otherwise
     *                specified.
     */
    virtual void set_default_formula_grammar(formula_grammar_t grammar) = 0;

    /**
     * Get current global formula grammar.  The import filter may use this
     * method to query the current global formula grammar.
     *
     * @return current default formula grammar.
     */
    virtual formula_grammar_t get_default_formula_grammar() const = 0;

    /**
     * Set the character set to use when parsing encoded string values.
     *
     * @param charset character set to use when parsing encoded string values.
     */
    virtual void set_character_set(character_set_t charset) = 0;
};

/**
 * This is an interface to allow the implementor to provide its own reference
 * address parsers, for both single cell references and cell range references.
 * The implementor may choose to provide a different parser depending of the
 * type of formula_ref_context_t argument given to the @ref
 * import_factory::get_reference_resolver() call.
 */
class ORCUS_DLLPUBLIC import_reference_resolver
{
public:
    virtual ~import_reference_resolver();

    /**
     * Resolve a textural representation of a single cell address.
     *
     * @param address single cell address string.
     *
     * @return structure containing the column and row positions of the
     *         address.
     *
     * @exception orcus::invalid_arg_error the string is not a valid
     *        single cell addreess.
     */
    virtual src_address_t resolve_address(std::string_view address) = 0;

    /**
     * Resolve a textural representation of a range address.  Note that a
     * string representing a valid single cell address should be considered a
     * valid range address.
     *
     * @param range range address string.
     *
     * @return structure containing the start and end positions of the range
     *         address.
     *
     * @exception invalid_arg_error the string is not a valid range addreess.
     */
    virtual src_range_t resolve_range(std::string_view range) = 0;
};

/**
 * This interface is the entry point for the import filter code to instantiate
 * other, more specialized interfaces.  The life cycles of any specialized
 * interfaces returned from this interface shall be managed by the implementor
 * of this interface.
 *
 * The implementer of this interface may wrap a backend document store that
 * needs to be populated.
 */
class ORCUS_DLLPUBLIC import_factory
{
public:
    virtual ~import_factory();

    /**
     * Obtain an optional interface for global settings, which the import filter
     * uses to specify global filter settings that may affect how certain values
     * and properties should be processed.  The implementor can use this
     * interface to decide how to process relevant values and properties.
     *
     * @return pointer to the global settings interface, or a @p nullptr if the
     *         implementor doesn't support it.
     */
    virtual import_global_settings* get_global_settings();

    /**
     * Obtain an optional interface for importing shared strings for string
     * cells.  Implementing this interface is required in order to import string
     * cell values.
     *
     * @return pointer to the shared strings interface, or a @p nullptr if the
     *         implementor doesn't support it.
     */
    virtual import_shared_strings* get_shared_strings();

    /**
     * Obtain an optional interface for importing global named expressions.
     *
     * Note that @ref import_sheet also provides the same interface, but its
     * interface is for importing sheet-local named expressions.
     *
     * @return pointer to the global named expression interface, or a @p nullptr
     *         if the implementor doesn't support it.
     */
    virtual import_named_expression* get_named_expression();

    /**
     * Obtain an optional interface for importing styles used to add formatting
     * properties to cell values.
     *
     * @return pointer to the styles interface, or a @p nullptr if the
     *         implementor doesn't support it.
     */
    virtual import_styles* get_styles();

    /**
     * Obtain an optional interface for resolving cell and cell-range references
     * from string values.
     *
     * @param cxt context in which the formula expression containing the
     *            references to be resolved occurs.
     *
     * @return pointer to the reference resolve interfance, or a @p nullptr if
     *         the implementor doesn't support it.
     */
    virtual import_reference_resolver* get_reference_resolver(formula_ref_context_t cxt);

    /**
     * Obtain an optional interface for pivot cache definition import for a
     * specified cache ID.  In case a pivot cache alrady exists for the passed
     * ID, the implementor should overwrite the existing cache with a brand-new
     * cache instance.
     *
     * @param cache_id numeric ID associated with the pivot cache.
     *
     * @return pointer to the pivot cache interface, or a @p nullptr if the
     *         implementor doesn't support pivot cache import.
     */
    virtual import_pivot_cache_definition* create_pivot_cache_definition(
        pivot_cache_id_t cache_id);

    /**
     * Obtain an optional interface for pivot cache records import for a
     * specified cache ID.
     *
     * @param cache_id numeric ID associated with the pivot cache.
     *
     * @return pointer to the pivot cache records interface, or a @p nullptr if
     *         the implementor doesn't support pivot cache import.
     */
    virtual import_pivot_cache_records* create_pivot_cache_records(
        pivot_cache_id_t cache_id);

    /**
     * Append a sheet with a specified sheet position index and name and return
     * an interface for importing its content.  The implementor can use a call
     * to this method as a signal to create and append a new sheet instance to
     * the document store.
     *
     * @param sheet_index position index of the sheet to be appended.  It is
     *                    0-based i.e. the first sheet to be appended will
     *                    have an index value of 0.
     * @param name sheet name.
     *
     * @return pointer to the sheet instance, or a @p nullptr if the implementor
     *         doesn't support it.  Note, however, that if the implementor
     *         doesn't support this interface, no cell values will get imported.
     */
    virtual import_sheet* append_sheet(sheet_t sheet_index, std::string_view name) = 0;

    /**
     * Get a sheet instance by name.  The import filter may use this method to
     * get access to an existing sheet after it has been created.
     *
     * @param name sheet name.
     *
     * @return pointer to the sheet instance whose name matches the name
     *         passed to this method. It returns a @p nullptr if no sheet
     *         instance exists by the specified name.
     */
    virtual import_sheet* get_sheet(std::string_view name) = 0;

    /**
     * Retrieve a sheet instance by a specified numerical sheet index.
     *
     * @param sheet_index sheet index.
     *
     * @return pointer to the sheet instance, or a @p nullptr if no sheet
     *         instance exists at the specified sheet index.
     */
    virtual import_sheet* get_sheet(sheet_t sheet_index) = 0;

    /**
     * The import filter calls this method after completing its import, to give
     * the implementor a chance to perform post-processing.
     */
    virtual void finalize() = 0;
};

}}}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
