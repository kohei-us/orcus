/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef ORCUS_SPREADSHEET_TYPES_HPP
#define ORCUS_SPREADSHEET_TYPES_HPP

#include "../env.hpp"

#include <cstdlib>
#include <cstdint>
#include <iosfwd>
#include <initializer_list>
#include <string_view>
#include <vector>

// NB: This header should only define primitive data types, enums and structs
// that have linkage to liborcus.

namespace orcus { namespace spreadsheet {

/** Row ID type. */
using row_t = int32_t;
/** Column ID type. */
using col_t = int32_t;
/** Sheet ID type. */
using sheet_t = int32_t;
/** Individual color element type. */
using color_elem_t = uint8_t;
/** Type for column width values.  Column width values are stored in twips. */
using col_width_t = uint16_t;
/** Type for row height values.  Row height values are stored in twips. */
using row_height_t = uint16_t;
/** Type for string ID's for string cells. */
using string_id_t = uint32_t;
/** Pivot cache ID type. */
using pivot_cache_id_t = uint32_t;

/**
 * Get the special column width value that represents the default column
 * width.  The value itself is not to be used as an actual width value.
 *
 * @return value that represents the default column width.
 */
ORCUS_DLLPUBLIC col_width_t get_default_column_width();

/**
 * Get the special row height value that represents the default row height.
 * The value itself is not to be used as an actual row height value.
 *
 * @return value that represents the default row height.
 */
ORCUS_DLLPUBLIC row_height_t get_default_row_height();

/**
 * Type of error value in cells.
 */
enum class error_value_t
{
    /**
     * Error type unknown, typically used as an initial error value or generic
     * default value.
     */
    unknown = 0,
    /** Null reference error, displayed as `#NULL!`. */
    null,
    /** Division-by-zero error, displayed as `#DIV/0`. */
    div0,
    /** Formula expression error, displayed as `#VALUE!`. */
    value,
    /** Reference error, displayed as `#REF!`. */
    ref,
    /** Invalid named-expression error, displayed as `#NAME?` */
    name,
    /** Invalid numeric value error, displayed as `#NUM!`. */
    num,
    /** No value is available error, displayed as `#N/A!`. */
    na
};

/**
 * Type of border direction, used to reference the position of a border in a
 * cell.
 */
enum class border_direction_t
{
    /** Unknown or uninitialized border direction value. */
    unknown = 0,
    /** Top border of a cell. */
    top,
    /** Bottom border of a cell. */
    bottom,
    /** Left border of a cell. */
    left,
    /** Right border of a cell. */
    right,
    /**
     * Cross-diagonal borders of a cell. This is equivalent of both
     * @p diagonal_bl_tr and @p diagonal_tl_br combined.
     */
    diagonal,
    /** Diagonal border of a cell that runs from bottom-left to top-right. */
    diagonal_bl_tr,
    /** Diagonal border of a cell that runs from top-left to bottom-right. */
    diagonal_tl_br
};

/**
 * Type of border style.
 */
enum class border_style_t
{
    unknown = 0,
    none,
    solid,
    dash_dot,
    dash_dot_dot,
    dashed,
    dotted,
    double_border,
    hair,
    medium,
    medium_dash_dot,
    medium_dash_dot_dot,
    medium_dashed,
    slant_dash_dot,
    thick,
    thin,
    double_thin,
    fine_dashed
};

/**
 * Type of fill pattern for cell background.
 */
enum class fill_pattern_t
{
    none = 0,
    solid,
    dark_down,
    dark_gray,
    dark_grid,
    dark_horizontal,
    dark_trellis,
    dark_up,
    dark_vertical,
    gray_0625,
    gray_125,
    light_down,
    light_gray,
    light_grid,
    light_horizontal,
    light_trellis,
    light_up,
    light_vertical,
    medium_gray
};

/**
 * Strikethrough style as applied to a cell value.
 *
 * @note This is specific to ODS format.
 */
enum class strikethrough_style_t
{
    none = 0,
    solid,
    dash,
    dot_dash,
    dot_dot_dash,
    dotted,
    long_dash,
    wave
};

/**
 * Strikethrough type as applied to a cell value.
 *
 * @note This is specific to ODS format.
 */
enum class strikethrough_type_t
{
    unknown = 0,
    none,
    single_type,
    double_type
};

/**
 * Width of strikethrough applied to a cell value.
 *
 * @note This is specific to ODS format.
 */
enum class strikethrough_width_t
{
    unknown = 0,
    width_auto,
    thin,
    medium,
    thick,
    bold
};

/**
 * Text used for strike-through.
 *
 * @note This is specific to ODS format.
 */
enum class strikethrough_text_t
{
    unknown = 0,
    /** `/` is used as the text. */
    slash,
    /** `X` is used as the text. */
    cross
};

/**
 * Type that specifies the grammar of a formula expression.  Each grammar
 * may exhibit a different set of syntax rules.
 */
enum class formula_grammar_t
{
    /** Grammar type is either unknown or unspecified. */
    unknown = 0,
    /** Grammar used by the Excel 2003 XML (aka XML Spreadsheet) format. */
    xls_xml,
    /** Grammar used by the Office Open XML spreadsheet format. */
    xlsx,
    /** Grammar used by the OpenDocument Spreadsheet format. */
    ods,
    /** Grammar used by the Gnumeric XML format. */
    gnumeric
};

/**
 * Type of formula expression.
 */
enum class formula_t
{
    /** Formula expression type unknown, or generic default value. */
    unknown = 0,
    /** Formula expression in an array of cells. */
    array,
    /** Formula expression in a data table. */
    data_table,
    /** Formula expression in a normal formula cell. */
    normal,
    /** Formula expression in a shared formula cell. */
    shared
};

/**
 * Formula reference context specifies the location where a formula
 * expression is used.  This is used mainly for those document formats that
 * make use of multiple formula reference syntaxes, such as ODS.
 */
enum class formula_ref_context_t
{
    /**
     * Default context, that is, the context that is NOT any of the other
     * contexts specified below.
     */
    global = 0,

    /** Base cell position of either a named range or expression. */
    named_expression_base,

    /**
     * Named range is a special case of named expression where the expression
     * consists of only one range token.
     */
    named_range,
};

/**
 * Type of policy on how to handle a formula cell with an erroneous expression
 * that has been parsed unsuccessfully.
 */
enum class formula_error_policy_t
{
    unknown,
    /** Loading of the document will be halted. */
    fail,
    /** The error cell will be skipped. */
    skip
};

/**
 * Style of an underline applied to a cell value.
 */
enum class underline_style_t
{
    /** Underline is absent. */
    none = 0,
    /** Line is continuous. */
    solid,
    /** Underline is dotted. */
    dotted,
    /** Underline is dashed. */
    dash,
    /** Underline consists of repeated long dash segments. */
    long_dash,
    /** Underline consists of repeated dot and dash segments. */
    dot_dash,
    /** Underline consists of repeated dot, dot and dash segments. */
    dot_dot_dash,
    /** Underline is waved. */
    wave
};

/**
 * Thickness of an underline.  When the enum value is either percent,
 * positive_integer, or positive_length, the actual value should be given
 * separately.
 *
 * @note For now ODS is the only format that makes use of this attribute
 * type.  In ODS, the corresponding attribute is referred to as "width".  The
 * automatic enum member corresponds with the "auto" text value, which could
 * not be used since it's a keyword in C++.
 */
enum class underline_thickness_t
{
    none = 0,
    automatic,
    bold,
    dash,
    medium,
    thick,
    thin,
    percent,
    positive_integer,
    positive_length
};

/**
 * Underline style related to how it is applied in relation to the spacing of
 * the text and the field it is in.
 *
 * @note ODS format makes extensive use of this attribute type.
 */
enum class underline_spacing_t
{
    /** Underline is applied to both words and spaces. */
    continuous = 0,
    /**
     * Underline is applied only to words, and not to the spaces between
     * them.
     */
    skip_white_space,
    /**
     * Underline is applied to the entire width of a field that houses the text.
     *
     * @note For now this is specific to Excel, and is referred to as
     *       "accounting" in Excel.
     */
    continuous_over_field,
};

/**
 * Underline style related to the number of vertically-stacked lines in an
 * underline.
 */
enum class underline_count_t
{
    none = 0,
    /** Only one line is used to form an underline. */
    single_count,
    /** Two vertically-stacked lines are used in an underline. */
    double_count
};

/**
 * Type of horizontal alignment applied to a cell content.
 */
enum class hor_alignment_t
{
    unknown = 0,
    left,
    center,
    right,
    justified,
    distributed,
    filled
};

/**
 * Type of vertical alignment applied to a cell content.
 */
enum class ver_alignment_t
{
    unknown = 0,
    top,
    middle,
    bottom,
    justified,
    distributed
};

/**
 * Cell format categories. The abbreviation "xf" stands for "cell format"
 * where the "x" is short for cell.
 */
enum class xf_category_t
{
    unknown,
    /** Direct cell format, also often referenced as xf. */
    cell,
    /** Cell format for named styles. */
    cell_style,
    /** Incremental cell format, also referenced as dxf. */
    differential,
};

/**
 * Type of data table.  A data table can be either of a single-variable
 * column, a single-variable row, or a double-variable type that uses both
 * column and row input cells.
 */
enum class data_table_type_t
{
    column,
    row,
    both
};

/**
 * Function type used in the totals row of a table.
 */
enum class totals_row_function_t
{
    none = 0,
    sum,
    minimum,
    maximum,
    average,
    count,
    count_numbers,
    standard_deviation,
    variance,
    custom
};

/**
 * Type of conditional format.
 */
enum class conditional_format_t
{
    unknown = 0,
    condition,
    date,
    formula,
    colorscale,
    databar,
    iconset
};

/**
 * Operator type associated with a conditional format rule.
 */
enum class condition_operator_t
{
    unknown = 0,
    equal,
    less,
    greater,
    greater_equal,
    less_equal,
    not_equal,
    between,
    not_between,
    duplicate,
    unique,
    top_n,
    bottom_n,
    above_average,
    below_average,
    above_equal_average,
    below_equal_average,
    contains_error,
    contains_no_error,
    begins_with,
    ends_with,
    contains,
    contains_blanks,
    not_contains,
    expression
};

/**
 * Type of a condition in a conditional format rule.  This is applicable only
 * when the type of a conditional format entry is either:
 *
 * @li @p colorscale,
 * @li @p databar or
 * @li @p iconset.
 */
enum class condition_type_t
{
    unknown = 0,
    value,
    automatic,
    max,
    min,
    formula,
    percent,
    percentile
};

/**
 * Type of a date condition when the type of a conditional format entry is
 * @p date.
 */
enum class condition_date_t
{
    unknown = 0,
    today,
    yesterday,
    tomorrow,
    last_7_days,
    this_week,
    next_week,
    last_week,
    this_month,
    next_month,
    last_month,
    this_year,
    next_year,
    last_year,
};

/**
 * Databar axis type, applicable only when the type of a conditional format
 * entry is @p databar.
 */
enum class databar_axis_t
{
    none = 0,
    middle,
    automatic
};

/**
 * Type of range grouping in a group field of a pivot table cache.
 */
enum class pivot_cache_group_by_t
{
    /**
     * Type of range grouping is unknown.
     *
     * This is an implicit default value of this type.
     */
    unknown = 0,
    /** Grouping on "days" for date values. */
    days,
    /** Grouping on "hours" for date values. */
    hours,
    /** Grouping on "minutes" for date values. */
    minutes,
    /** Grouping on "months" for date values. */
    months,
    /** Grouping on "quarters" for date values. */
    quarters,
    /** Grouping by numeric ranges for numeric values. */
    range,
    /** Grouping on "seconds" for date values. */
    seconds,
    /** Grouping on "years" for date values. */
    years
};

/**
 * Stores a 2-dimensional cell address.
 */
struct address_t
{
    row_t row;
    col_t column;
};

/**
 * Stores the size of a range of a spreadsheet.
 */
struct range_size_t
{
    row_t rows;
    col_t columns;
};

/**
 * Stores a 2-dimensional cell range by storing the positions of the top-left
 * and bottom-right corners of the range.
 */
struct range_t
{
    address_t first;
    address_t last;
};

/**
 * Stores 3-dimensional cell address.  The 'src' abbreviation stands for
 * sheet-row-column.
 */
struct src_address_t
{
    sheet_t sheet;
    row_t row;
    col_t column;
};

/**
 * Stores 3-dimensional cell range address.  The 'src' abbreviation stands for
 * sheet-row-column.
 */
struct src_range_t
{
    src_address_t first;
    src_address_t last;
};

/**
 * Convert a 3-dimensional cell address to a 2-dimensional counterpart by
 * dropping the sheet index.
 */
ORCUS_DLLPUBLIC address_t to_rc_address(const src_address_t& r);

/**
 * Convert a 3-dimensional cell range address to a 2-dimensional counterpart
 * by dropping the sheet indices.
 */
ORCUS_DLLPUBLIC range_t to_rc_range(const src_range_t& r);

ORCUS_DLLPUBLIC bool operator== (const address_t& left, const address_t& right);
ORCUS_DLLPUBLIC bool operator!= (const address_t& left, const address_t& right);

ORCUS_DLLPUBLIC bool operator== (const src_address_t& left, const src_address_t& right);
ORCUS_DLLPUBLIC bool operator!= (const src_address_t& left, const src_address_t& right);

ORCUS_DLLPUBLIC bool operator== (const range_t& left, const range_t& right);
ORCUS_DLLPUBLIC bool operator!= (const range_t& left, const range_t& right);

ORCUS_DLLPUBLIC bool operator== (const src_range_t& left, const src_range_t& right);
ORCUS_DLLPUBLIC bool operator!= (const src_range_t& left, const src_range_t& right);

ORCUS_DLLPUBLIC bool operator< (const range_t& left, const range_t& right);
ORCUS_DLLPUBLIC bool operator> (const range_t& left, const range_t& right);

ORCUS_DLLPUBLIC range_t& operator+= (range_t& left, const address_t& right);
ORCUS_DLLPUBLIC range_t& operator-= (range_t& left, const address_t& right);

ORCUS_DLLPUBLIC std::ostream& operator<< (std::ostream& os, const address_t& v);
ORCUS_DLLPUBLIC std::ostream& operator<< (std::ostream& os, const src_address_t& v);
ORCUS_DLLPUBLIC std::ostream& operator<< (std::ostream& os, const range_t& v);

/**
 * Stores a color value in RGB format.
 */
struct color_rgb_t
{
    color_elem_t red;
    color_elem_t green;
    color_elem_t blue;
};

/**
 * Convert a string representation of a totals row function name to its
 * equivalent enum value.
 *
 * @param s string value for totals row function name.
 *
 * @return enum value representing the totals row function.
 */
ORCUS_DLLPUBLIC totals_row_function_t to_totals_row_function_enum(std::string_view s);

/**
 * Convert a string representation of a pivot cache group-by type to its
 * equivalent enum value.
 *
 * @param s string value for pivot cache group-by type.
 *
 * @return enum value representing the pivot cache group-by type.
 */
ORCUS_DLLPUBLIC pivot_cache_group_by_t to_pivot_cache_group_by_enum(std::string_view s);

/**
 * Convert a string representation of a error value to its equivalent enum
 * value.
 *
 * @param s error value string.
 *
 * @return enum value representing the error value.
 */
ORCUS_DLLPUBLIC error_value_t to_error_value_enum(std::string_view s);

/**
 * Convert a string representation of a RGB value to an equivalent struct
 * value.  The string representation is expected to be a 6 digit hexadecimal
 * value string that may or may not be prefixed with a '#'.
 *
 * @param s string representation of the RGB value.
 *
 * @return struct value representing an RGB value.
 */
ORCUS_DLLPUBLIC color_rgb_t to_color_rgb(std::string_view s);

/**
 * Convert a color name to an RGB value.  It supports SVG 1.0 color keyword
 * names minus those gray colors with 'grey' spelling variants.  Note that
 * the name must be all in lowercase.
 *
 * @param s color name.
 *
 * @return struct value representing an RGB value.
 */
ORCUS_DLLPUBLIC color_rgb_t to_color_rgb_from_name(std::string_view s);

/**
 * Convert a formula error policy name to its enum value equivalent.
 *
 * @param s policy name.
 *
 * @return enum value equivalent for the original error policy name.
 */
ORCUS_DLLPUBLIC formula_error_policy_t to_formula_error_policy(std::string_view s);

ORCUS_DLLPUBLIC std::ostream& operator<< (std::ostream& os, error_value_t ev);
ORCUS_DLLPUBLIC std::ostream& operator<< (std::ostream& os, border_style_t border);
ORCUS_DLLPUBLIC std::ostream& operator<< (std::ostream& os, formula_grammar_t grammar);
ORCUS_DLLPUBLIC std::ostream& operator<< (std::ostream& os, underline_style_t uline);
ORCUS_DLLPUBLIC std::ostream& operator<< (std::ostream& os, underline_thickness_t ulwidth);
ORCUS_DLLPUBLIC std::ostream& operator<< (std::ostream& os, underline_spacing_t ulmode);
ORCUS_DLLPUBLIC std::ostream& operator<< (std::ostream& os, underline_count_t ultype);
ORCUS_DLLPUBLIC std::ostream& operator<< (std::ostream& os, hor_alignment_t halign);
ORCUS_DLLPUBLIC std::ostream& operator<< (std::ostream& os, ver_alignment_t valign);
ORCUS_DLLPUBLIC std::ostream& operator<< (std::ostream& os, const color_rgb_t& color);
ORCUS_DLLPUBLIC std::ostream& operator<< (std::ostream& os, const fill_pattern_t& fill);
ORCUS_DLLPUBLIC std::ostream& operator<< (std::ostream& os, const strikethrough_style_t& ss);
ORCUS_DLLPUBLIC std::ostream& operator<< (std::ostream& os, const strikethrough_type_t& st);
ORCUS_DLLPUBLIC std::ostream& operator<< (std::ostream& os, const strikethrough_width_t& sw);
ORCUS_DLLPUBLIC std::ostream& operator<< (std::ostream& os, const strikethrough_text_t& st);

}}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
