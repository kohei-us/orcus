/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef ORCUS_SPREADSHEET_TYPES_HPP
#define ORCUS_SPREADSHEET_TYPES_HPP

#include "orcus/env.hpp"
#include <cstdlib>
#include <cstdint>
#include <iosfwd>
#include <initializer_list>

// NB: This header should only use primitive data types and enums.

namespace orcus { namespace spreadsheet {

typedef int32_t row_t;
typedef int32_t col_t;
typedef int32_t sheet_t;
typedef uint8_t color_elem_t;
typedef uint16_t col_width_t;
typedef uint16_t row_height_t;

typedef uint32_t pivot_cache_id_t;

ORCUS_DLLPUBLIC col_width_t get_default_column_width();
ORCUS_DLLPUBLIC row_height_t get_default_row_height();

enum class error_value_t
{
    unknown = 0,
    null,         // #NULL!
    div0,         // #DIV/0!
    value,        // #VALUE!
    ref,          // #REF!
    name,         // #NAME?
    num,          // #NUM!
    na            // #N/A!
};

enum class border_direction_t
{
    unknown = 0,
    top,
    bottom,
    left,
    right,
    diagonal,
    diagonal_bl_tr,
    diagonal_tl_br
};

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

enum class strikethrough_type_t
{
    unknown = 0,
    none,
    single,
    double_type
};

enum class strikethrough_width_t
{
    unknown = 0,
    width_auto,
    thin,
    medium,
    thick,
    bold
};

enum class strikethrough_text_t
{
    unknown = 0,
    slash,
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

enum class formula_t
{
    unknown = 0,
    array,
    data_table,
    normal,
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

    /** base cell position of either a named range or expression. */
    named_expression_base,

    /**
     * named range is a special case of named expression where the expression
     * consists of only one range token.
     */
    named_range,
};

enum class underline_t
{
    none = 0,
    single_line,
    single_accounting, // unique to xlsx
    double_line,
    double_accounting, // unique to xlsx
    dotted,
    dash,
    long_dash,
    dot_dash,
    dot_dot_dot_dash,
    wave
};

enum class underline_width_t
{
    none = 0,
    normal,
    bold,
    thin,
    medium,
    thick,
    positive_integer,
    percent,
    positive_length
};

enum class underline_mode_t
{
    continuos = 0,
    skip_white_space
};

enum class underline_type_t
{
    none = 0,
    single,
    double_type            //necessary to not call it "double", since it is a reserved word
};

struct underline_attrs_t
{
    underline_t underline_style;
    underline_width_t underline_width;
    underline_mode_t underline_mode;
    underline_type_t underline_type;
};

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

enum class databar_axis_t
{
    none = 0,
    middle,
    automatic
};

enum class pivot_cache_group_by_t
{
    unknown = 0,
    days,     // grouping on "days" for date values.
    hours,    // grouping on "hours" for date values.
    minutes,  // grouping on "minutes" for date values.
    months,   // grouping on "months" for date values.
    quarters, // grouping on "quarters" for date values.
    range,    // grouping by numeric ranges for numeric values.
    seconds,  // grouping on "seconds" for date values.
    years     // grouping on "years" for date values.
};

struct address_t
{
    row_t row;
    col_t column;
};

struct range_size_t
{
    row_t rows;
    col_t columns;
};

struct range_t
{
    address_t first;
    address_t last;
};

/**
 * Stores 3-dimensional cell address.  The 'src' stands for
 * sheet-row-column.
 */
struct src_address_t
{
    sheet_t sheet;
    row_t row;
    col_t column;
};

/**
 * Stores 3-dimensional range address.  The 'src' stands for
 * sheet-row-column.
 */
struct src_range_t
{
    src_address_t first;
    src_address_t last;
};

ORCUS_DLLPUBLIC address_t to_rc_address(const src_address_t& r);
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

struct ORCUS_DLLPUBLIC color_rgb_t
{
    color_elem_t red;
    color_elem_t green;
    color_elem_t blue;

    color_rgb_t();
    color_rgb_t(std::initializer_list<color_elem_t> vs);
    color_rgb_t(const color_rgb_t& other);
    color_rgb_t(color_rgb_t&& other);

    color_rgb_t& operator= (const color_rgb_t& other);
};

/**
 * Convert a string representation of a totals row function name to its
 * equivalent enum value.
 *
 * @param p pointer to the string buffer.
 * @param n size of the string buffer.
 *
 * @return enum value representing the totals row function.
 */
ORCUS_DLLPUBLIC totals_row_function_t to_totals_row_function_enum(const char* p, size_t n);

/**
 * Convert a string representation of a pivot cache group-by type to its
 * equivalent enum value.
 *
 * @param p pointer to the string buffer.
 * @param n size of the string buffer.
 *
 * @return enum value representing the pivot cache group-by type.
 */
ORCUS_DLLPUBLIC pivot_cache_group_by_t to_pivot_cache_group_by_enum(const char* p, size_t n);

/**
 * Convert a string representation of a error value to its equivalent enum
 * value.
 *
 * @param p pointer to the string buffer.
 * @param n size of the string buffer.
 *
 * @return enum value representing the error value.
 */
ORCUS_DLLPUBLIC error_value_t to_error_value_enum(const char* p, size_t n);

/**
 * Convert a string representation of a RGB value to an equivalent struct
 * value.  The string representation is expected to be a 6 digit hexadecimal
 * value string that may or may not be prefixed with a '#'.
 *
 * @param p pointer to the string buffer that stores the string
 *          representation of the RGB value.
 * @param n length of the buffer.
 *
 * @return struct value representing an RGB value.
 */
ORCUS_DLLPUBLIC color_rgb_t to_color_rgb(const char* p, size_t n);

/**
 * Convert a color name to an RGB value.  It supports SVG 1.0 color keyword
 * names minus those gray colors with 'grey' spelling variants.  Note that
 * the name must be all in lowercase.
 *
 * @param p pointer to the string buffer that stores the color name.
 * @param n length of the buffer.
 *
 * @return struct value representing an RGB value.
 */
ORCUS_DLLPUBLIC color_rgb_t to_color_rgb_from_name(const char* p, size_t n);

ORCUS_DLLPUBLIC std::ostream& operator<< (std::ostream& os, error_value_t ev);
ORCUS_DLLPUBLIC std::ostream& operator<< (std::ostream& os, formula_grammar_t grammar);
ORCUS_DLLPUBLIC std::ostream& operator<< (std::ostream& os, const color_rgb_t& color);

}}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
