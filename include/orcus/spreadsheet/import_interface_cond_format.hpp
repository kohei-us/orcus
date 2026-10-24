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
 * Interface for importing a single value of a conditional format rule entry.
 *
 * A value is either an operand of the rule's operator, or a threshold of a
 * color scale, data bar or icon set.  It consists of a formula, number or
 * string, a value type, and an optional color.
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
     * Set the type of the value.  This is only applicable when the type of
     * the parent entry is either @p colorscale, @p databar or @p iconset.
     *
     * @param type Type of the value.
     */
    virtual void set_type(cond_format_value_type_t type) = 0;

    /**
     * Set the color of the value.  This is only applicable when the type
     * of the parent entry is @p colorscale.
     */
    virtual void set_color(color_elem_t alpha, color_elem_t red, color_elem_t green, color_elem_t blue) = 0;

    /**
     * Commit the value to the parent entry.
     */
    virtual void commit() = 0;
};

/**
 * Interface for importing a single rule entry of a conditional format.
 *
 * Each rule entry consists of:
 *
 * @li a type of rule,
 * @li zero or more rule properties, and
 * @li zero or more values depending on the rule type.
 */
class ORCUS_DLLPUBLIC import_cond_format_entry
{
public:
    virtual ~import_cond_format_entry();

    /**
     * Set the type of the rule entry.
     *
     * @param type Type of the rule entry.
     */
    virtual void set_type(cond_format_rule_t type) = 0;

    /**
     * Set the operator used for the rule entry.  This is only applicable
     * when the type of the entry is @p condition.
     *
     * @param op Operator used for the rule entry.
     */
    virtual void set_operator(cond_format_operator_t op) = 0;

    /**
     * Set the type of the date condition.  This is only applicable when the
     * type of the entry is @p date.
     *
     * @param date Type of the date condition.
     */
    virtual void set_date(cond_format_date_t date) = 0;

    /**
     * Set the ID of the cell format to apply when the rule entry is met.
     *
     * @todo In OOXML the style is stored as dxf and in ODF as named style.
     *
     * @param xf ID of the cell format.
     */
    virtual void set_xf_id(std::size_t xf) = 0;

    /**
     * Set whether or not to show the value in the cell.  This is only
     * applicable when the type of the entry is @p databar, @p iconset or
     * @p colorscale.
     *
     * @param show Whether or not to show the value in the cell.
     */
    virtual void set_show_value(bool show) = 0;

    /**
     * Set the rank value.  This is only applicable when the operator of the
     * entry is either @p top_n or @p bottom_n.
     *
     * @param rank Rank value.
     */
    virtual void set_rank(std::string_view rank) = 0;

    /**
     * Set the text value.  This is only applicable when the operator of the
     * entry is one of the text operators such as @p contains, @p not_contains,
     * @p begins_with and @p ends_with.
     *
     * @param text Text value.
     */
    virtual void set_text(std::string_view text) = 0;

    /**
     * Set the number of standard deviations.  This is only applicable when
     * the operator of the entry is one of the average operators.
     *
     * @param sd Number of standard deviations.
     */
    virtual void set_std_dev(std::string_view sd) = 0;

    /**
     * Set whether or not to use a gradient for the data bar.  This is only
     * applicable when the type of the entry is @p databar.
     *
     * @param gradient Whether or not to use a gradient.
     */
    virtual void set_databar_gradient(bool gradient) = 0;

    /**
     * Set the position of the 0 axis of the data bar.  This is only
     * applicable when the type of the entry is @p databar.
     *
     * @param axis Position of the 0 axis.
     */
    virtual void set_databar_axis(databar_axis_t axis) = 0;

    /**
     * Set the data bar color for positive values.  This is only applicable
     * when the type of the entry is @p databar.
     */
    virtual void set_databar_color_positive(color_elem_t alpha, color_elem_t red, color_elem_t green, color_elem_t blue) = 0;

    /**
     * Set the data bar color for negative values.  This is only applicable
     * when the type of the entry is @p databar.
     */
    virtual void set_databar_color_negative(color_elem_t alpha, color_elem_t red, color_elem_t green, color_elem_t blue) = 0;

    /**
     * Set the minimum length of the data bar.  This is only applicable when
     * the type of the entry is @p databar.
     *
     * @param length Minimum length of the data bar.
     */
    virtual void set_databar_min_length(double length) = 0;

    /**
     * Set the maximum length of the data bar.  This is only applicable when
     * the type of the entry is @p databar.
     *
     * @param length Maximum length of the data bar.
     */
    virtual void set_databar_max_length(double length) = 0;

    /**
     * Set the name of the icons to use.  This is only applicable when the
     * type of the entry is @p iconset.
     *
     * @param name Name of the icons.
     */
    virtual void set_icon_name(std::string_view name) = 0;

    /**
     * Set whether or not to use the icons in reverse order.  This is only
     * applicable when the type of the entry is @p iconset.
     *
     * @param reverse Whether or not to use the icons in reverse order.
     */
    virtual void set_iconset_reverse(bool reverse) = 0;

    /**
     * Start importing a new value for the rule entry.
     *
     * The implementor should initialize the internal buffer to store the
     * value data when this method is called.
     *
     * @return Interface for importing the new value.
     *
     * @note Note that the import_cond_format_entry implementer <i>must</i>
     *       return a non-null pointer.
     */
    virtual import_cond_format_value* start_value() = 0;

    /**
     * Commit the rule entry to the parent conditional format.
     */
    virtual void commit() = 0;
};

/**
 * Interface for importing a single conditional format.
 *
 * In general, a single conditional format consists of:
 *
 * @li one or more cell ranges the format is applied to, and
 * @li one or more rule entries.
 *
 * The flow of the import process varies depending on the type of the
 * conditional format being imported.  The following is an example of
 * importing a conditional format that consists of a rule that applies a
 * format when the cell value is greater than 2:
 *
 * @code{.cpp}
 * import_sheet* sheet = ... ;
 *
 * auto* cf = sheet->start_cond_format();
 * cf->append_range(range); // A2:A13
 *
 * auto* entry = cf->start_entry();
 * entry->set_type(cond_format_rule_t::condition);
 * entry->set_operator(cond_format_operator_t::greater);
 * entry->set_xf_id(14); // apply differential format (dxf) whose ID is 14
 *
 * auto* value = entry->start_value();
 * value->set_formula("2");
 * value->commit();
 *
 * entry->commit();
 * cf->commit();
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
     * Start importing a new rule entry for the conditional format.
     *
     * The implementor should initialize the internal buffer to store the
     * rule entry data when this method is called.
     *
     * @return Interface for importing the new rule entry.
     *
     * @note Note that the import_cond_format implementer <i>must</i> return a
     *       non-null pointer.
     */
    virtual import_cond_format_entry* start_entry() = 0;

    /**
     * Commit the conditional format to the destination store.
     */
    virtual void commit() = 0;
};

}}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
