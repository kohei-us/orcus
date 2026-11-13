/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "types.hpp"
#include "../types.hpp"
#include "../env.hpp"

// NB: This header must not depend on ixion, as it needs to be usable for
// those clients that provide their own formula engine.  Other headers in
// the orcus::spreadsheet namespace may depend on ixion.

namespace orcus { namespace spreadsheet { namespace iface {

/**
 * Interface for importing a single value of a conditional format rule.
 *
 * A value is either an operand of a condition rule, or a threshold of a
 * color scale, data bar or icon set rule.  It consists of a formula, number
 * or string, a value type, and an optional color.
 */
class ORCUS_DLLPUBLIC import_cond_format_value
{
public:
    virtual ~import_cond_format_value();

    /**
     * Set the content of the value as a formula, number or string.
     *
     * @param formula Formula, number or string.
     */
    virtual void set_formula(std::string_view formula) = 0;

    /**
     * Set the type of the value.  This is only applicable when the value is
     * a threshold of a color scale, data bar or icon set rule.
     *
     * @param type Type of the value.
     */
    virtual void set_type(cond_format_value_type_t type) = 0;

    /**
     * Set the color of the value.  This is only applicable when the value
     * is a threshold of a color scale rule.
     */
    virtual void set_color(color_elem_t alpha, color_elem_t red, color_elem_t green, color_elem_t blue) = 0;

    /**
     * Commit the value to the parent rule.
     */
    virtual void commit() = 0;
};

/**
 * Interface for importing a condition rule of a conditional format.  A
 * condition rule applies a cell format when the condition expressed by its
 * operator and operands is met.
 */
class ORCUS_DLLPUBLIC import_cond_format_condition
{
public:
    virtual ~import_cond_format_condition();

    /**
     * Set the operator of the condition.
     *
     * @param op Operator of the condition.
     */
    virtual void set_operator(cond_format_operator_t op) = 0;

    /**
     * Set the ID of the differential cell format (dxf) to apply when the
     * condition is met.
     *
     * @param dxf ID of the differential cell format.
     */
    virtual void set_dxf_id(std::size_t dxf) = 0;

    /**
     * Set the rank value.  This is only applicable when the operator is one
     * of @p top_n, @p bottom_n, @p top_percent or @p bottom_percent.
     *
     * @param rank Rank value.
     */
    virtual void set_rank(std::string_view rank) = 0;

    /**
     * Set the text value.  This is only applicable when the operator is one
     * of the text operators such as @p contains, @p not_contains,
     * @p begins_with and @p ends_with.
     *
     * @param text Text value.
     */
    virtual void set_text(std::string_view text) = 0;

    /**
     * Set the number of standard deviations.  This is only applicable when
     * the operator is one of the average operators.
     *
     * @param sd Number of standard deviations.
     */
    virtual void set_std_dev(std::string_view sd) = 0;

    /**
     * Start importing a new operand of the condition.
     *
     * The implementor should initialize the internal buffer to store the
     * value data when this method is called.
     *
     * @return Interface for importing the new value.
     *
     * @note Note that the import_cond_format_condition implementer
     *       <i>must</i> return a non-null pointer.
     */
    virtual import_cond_format_value* start_value() = 0;

    /**
     * Commit the condition rule to the parent conditional format.
     */
    virtual void commit() = 0;
};

/**
 * Interface for importing a date rule of a conditional format.  A date rule
 * applies a cell format when the cell value falls within a date period.
 */
class ORCUS_DLLPUBLIC import_cond_format_date
{
public:
    virtual ~import_cond_format_date();

    /**
     * Set the date period of the rule.
     *
     * @param period Date period of the rule.
     */
    virtual void set_period(cond_format_date_t period) = 0;

    /**
     * Set the ID of the differential cell format (dxf) to apply when the
     * cell value falls within the date period.
     *
     * @param dxf ID of the differential cell format.
     */
    virtual void set_dxf_id(std::size_t dxf) = 0;

    /**
     * Commit the date rule to the parent conditional format.
     */
    virtual void commit() = 0;
};

/**
 * Interface for importing a color scale rule of a conditional format.  A
 * color scale rule colors each cell by interpolating between two or more
 * thresholds, each of which carries a color.
 */
class ORCUS_DLLPUBLIC import_cond_format_colorscale
{
public:
    virtual ~import_cond_format_colorscale();

    /**
     * Start importing a new threshold of the color scale.
     *
     * The implementor should initialize the internal buffer to store the
     * value data when this method is called.
     *
     * @return Interface for importing the new value.
     *
     * @note Note that the import_cond_format_colorscale implementer
     *       <i>must</i> return a non-null pointer.
     */
    virtual import_cond_format_value* start_value() = 0;

    /**
     * Commit the color scale rule to the parent conditional format.
     */
    virtual void commit() = 0;
};

/**
 * Interface for importing a data bar rule of a conditional format.  A data
 * bar rule draws a bar in each cell whose length reflects the cell value
 * relative to two thresholds.
 */
class ORCUS_DLLPUBLIC import_cond_format_databar
{
public:
    virtual ~import_cond_format_databar();

    /**
     * Set whether or not to show the value in the cell.
     *
     * @param show Whether or not to show the value in the cell.
     */
    virtual void set_show_value(bool show) = 0;

    /**
     * Set whether or not to use a gradient for the data bar.
     *
     * @param gradient Whether or not to use a gradient.
     */
    virtual void set_gradient(bool gradient) = 0;

    /**
     * Set the position of the 0 axis of the data bar.
     *
     * @param axis Position of the 0 axis.
     */
    virtual void set_axis(databar_axis_t axis) = 0;

    /**
     * Set the data bar color for positive values.
     */
    virtual void set_color_positive(
        color_elem_t alpha, color_elem_t red, color_elem_t green, color_elem_t blue) = 0;

    /**
     * Set the data bar color for negative values.
     */
    virtual void set_color_negative(
        color_elem_t alpha, color_elem_t red, color_elem_t green, color_elem_t blue) = 0;

    /**
     * Set the minimum length of the data bar.
     *
     * @param length Minimum length of the data bar.
     */
    virtual void set_min_length(double length) = 0;

    /**
     * Set the maximum length of the data bar.
     *
     * @param length Maximum length of the data bar.
     */
    virtual void set_max_length(double length) = 0;

    /**
     * Start importing a new threshold of the data bar.  A data bar has
     * exactly two thresholds.
     *
     * The implementor should initialize the internal buffer to store the
     * value data when this method is called.
     *
     * @return Interface for importing the new value.
     *
     * @note Note that the import_cond_format_databar implementer
     *       <i>must</i> return a non-null pointer.
     */
    virtual import_cond_format_value* start_value() = 0;

    /**
     * Commit the data bar rule to the parent conditional format.
     */
    virtual void commit() = 0;
};

/**
 * Interface for importing an icon set rule of a conditional format.  An
 * icon set rule shows an icon in each cell chosen by comparing the cell
 * value against a series of thresholds.
 */
class ORCUS_DLLPUBLIC import_cond_format_iconset
{
public:
    virtual ~import_cond_format_iconset();

    /**
     * Set whether or not to show the value in the cell.
     *
     * @param show Whether or not to show the value in the cell.
     */
    virtual void set_show_value(bool show) = 0;

    /**
     * Set the set of icons to use.
     *
     * @param icons Set of icons to use.
     */
    virtual void set_icon_set(icon_set_t icons) = 0;

    /**
     * Set whether or not to use the icons in reverse order.
     *
     * @param reverse Whether or not to use the icons in reverse order.
     */
    virtual void set_reverse(bool reverse) = 0;

    /**
     * Start importing a new threshold of the icon set.  Each threshold
     * corresponds to one icon.
     *
     * The implementor should initialize the internal buffer to store the
     * value data when this method is called.
     *
     * @return Interface for importing the new value.
     *
     * @note Note that the import_cond_format_iconset implementer
     *       <i>must</i> return a non-null pointer.
     */
    virtual import_cond_format_value* start_value() = 0;

    /**
     * Commit the icon set rule to the parent conditional format.
     */
    virtual void commit() = 0;
};

/**
 * Interface for importing a single conditional format.
 *
 * In general, a single conditional format consists of:
 *
 * @li one or more cell ranges the format is applied to, and
 * @li one or more rules, each of which is one of the five rule types.
 *
 * Each rule type has its own interface, obtained from the matching
 * <code>start_*</code> method.  The following examples show how to import
 * each type of rule.
 *
 * A condition rule that applies a cell format when the cell value is
 * greater than 2:
 *
 * @code{.cpp}
 * import_sheet* sheet = ... ;
 *
 * auto* cf = sheet->start_cond_format();
 * cf->append_range(range); // A2:A13
 *
 * auto* rule = cf->start_condition();
 * rule->set_operator(cond_format_operator_t::greater);
 * rule->set_dxf_id(14); // apply differential format (dxf) whose ID is 14
 *
 * auto* value = rule->start_value();
 * value->set_formula("2");
 * value->commit();
 *
 * rule->commit();
 * cf->commit();
 * @endcode
 *
 * A date rule that applies a cell format when the cell value falls within
 * the last 7 days:
 *
 * @code{.cpp}
 * auto* rule = cf->start_date();
 * rule->set_period(cond_format_date_t::last_7_days);
 * rule->set_dxf_id(13);
 * rule->commit();
 * @endcode
 *
 * A color scale rule that interpolates between red at the minimum value
 * and green at the maximum value:
 *
 * @code{.cpp}
 * auto* rule = cf->start_colorscale();
 *
 * auto* value = rule->start_value();
 * value->set_type(cond_format_value_type_t::min);
 * value->set_color(0xFF, 0xFF, 0x00, 0x00);
 * value->commit();
 *
 * value = rule->start_value();
 * value->set_type(cond_format_value_type_t::max);
 * value->set_color(0xFF, 0x00, 0xFF, 0x00);
 * value->commit();
 *
 * rule->commit();
 * @endcode
 *
 * A data bar rule with a blue bar spanning from the minimum value to the
 * maximum value:
 *
 * @code{.cpp}
 * auto* rule = cf->start_databar();
 * rule->set_color_positive(0xFF, 0x63, 0x8E, 0xC6);
 * rule->set_color_negative(0xFF, 0x63, 0x8E, 0xC6);
 *
 * auto* value = rule->start_value();
 * value->set_type(cond_format_value_type_t::min);
 * value->commit();
 *
 * value = rule->start_value();
 * value->set_type(cond_format_value_type_t::max);
 * value->commit();
 *
 * rule->commit();
 * @endcode
 *
 * An icon set rule with three arrows, switching icons at the 33rd and 67th
 * percentiles:
 *
 * @code{.cpp}
 * auto* rule = cf->start_iconset();
 * rule->set_icon_set(icon_set_t::three_arrows);
 *
 * auto* value = rule->start_value();
 * value->set_type(cond_format_value_type_t::percent);
 * value->set_formula("0");
 * value->commit();
 *
 * value = rule->start_value();
 * value->set_type(cond_format_value_type_t::percent);
 * value->set_formula("33");
 * value->commit();
 *
 * value = rule->start_value();
 * value->set_type(cond_format_value_type_t::percent);
 * value->set_formula("67");
 * value->commit();
 *
 * rule->commit();
 * @endcode
 */
class ORCUS_DLLPUBLIC import_cond_format
{
public:
    virtual ~import_cond_format();

    /**
     * Append a cell range the conditional format is applied to.
     *
     * @param range Cell range the conditional format is applied to.
     */
    virtual void append_range(const range_t& range) = 0;

    /**
     * Start importing a new condition rule.
     *
     * The implementor should initialize the internal buffer to store the
     * rule data when this method is called.
     *
     * @return Interface for importing the new condition rule.
     *
     * @note Note that the import_cond_format implementer <i>must</i> return a
     *       non-null pointer.
     */
    virtual import_cond_format_condition* start_condition() = 0;

    /**
     * Start importing a new date rule.
     *
     * The implementor should initialize the internal buffer to store the
     * rule data when this method is called.
     *
     * @return Interface for importing the new date rule.
     *
     * @note Note that the import_cond_format implementer <i>must</i> return a
     *       non-null pointer.
     */
    virtual import_cond_format_date* start_date() = 0;

    /**
     * Start importing a new color scale rule.
     *
     * The implementor should initialize the internal buffer to store the
     * rule data when this method is called.
     *
     * @return Interface for importing the new color scale rule.
     *
     * @note Note that the import_cond_format implementer <i>must</i> return a
     *       non-null pointer.
     */
    virtual import_cond_format_colorscale* start_colorscale() = 0;

    /**
     * Start importing a new data bar rule.
     *
     * The implementor should initialize the internal buffer to store the
     * rule data when this method is called.
     *
     * @return Interface for importing the new data bar rule.
     *
     * @note Note that the import_cond_format implementer <i>must</i> return a
     *       non-null pointer.
     */
    virtual import_cond_format_databar* start_databar() = 0;

    /**
     * Start importing a new icon set rule.
     *
     * The implementor should initialize the internal buffer to store the
     * rule data when this method is called.
     *
     * @return Interface for importing the new icon set rule.
     *
     * @note Note that the import_cond_format implementer <i>must</i> return a
     *       non-null pointer.
     */
    virtual import_cond_format_iconset* start_iconset() = 0;

    /**
     * Commit the conditional format to the destination store.
     */
    virtual void commit() = 0;
};

}}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
