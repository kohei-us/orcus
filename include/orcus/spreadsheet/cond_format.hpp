/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "types.hpp"
#include "document_types.hpp"
#include "../env.hpp"

#include <optional>
#include <string_view>
#include <variant>
#include <vector>

#include <ixion/address.hpp>

namespace orcus { namespace spreadsheet {

/**
 * Stores a single value of a conditional format rule.  A value is either an
 * operand of a condition rule, or a threshold of a color scale, data bar or
 * icon set rule.
 */
struct ORCUS_SPM_DLLPUBLIC cond_format_value_t
{
    /** Formula, number or string that defines the value. */
    std::string_view formula;

    /** Type of the value, when the value is a threshold. */
    cond_format_value_type_t type = cond_format_value_type_t::unknown;

    /** Color of the value, when the value is a threshold of a color scale. */
    std::optional<color_t> color;

    cond_format_value_t();
    cond_format_value_t(const cond_format_value_t& other);
    cond_format_value_t(cond_format_value_t&& other);
    ~cond_format_value_t();

    cond_format_value_t& operator=(cond_format_value_t other);

    void swap(cond_format_value_t& other) noexcept;
};

/**
 * Stores a condition rule of a conditional format, which applies a cell
 * format when the condition expressed by its operator and operands is met.
 */
struct ORCUS_SPM_DLLPUBLIC cond_format_condition_rule_t
{
    /** Operator of the condition. */
    cond_format_operator_t op = cond_format_operator_t::unknown;

    /** ID of the differential cell format to apply when the condition is met. */
    std::optional<std::size_t> dxf_id;

    /** Rank value, when the operator is either top_n or bottom_n. */
    std::string_view rank;

    /** Text value, when the operator is one of the text operators. */
    std::string_view text;

    /** Number of standard deviations, when the operator is one of the average operators. */
    std::string_view std_dev;

    /** Operands of the condition. */
    std::vector<cond_format_value_t> values;

    cond_format_condition_rule_t();
    cond_format_condition_rule_t(const cond_format_condition_rule_t& other);
    cond_format_condition_rule_t(cond_format_condition_rule_t&& other);
    ~cond_format_condition_rule_t();

    cond_format_condition_rule_t& operator=(cond_format_condition_rule_t other);

    void swap(cond_format_condition_rule_t& other) noexcept;
};

/**
 * Stores a date rule of a conditional format, which applies a cell format
 * when the cell value falls within a date period.
 */
struct ORCUS_SPM_DLLPUBLIC cond_format_date_rule_t
{
    /** Date period of the rule. */
    cond_format_date_t period = cond_format_date_t::unknown;

    /** ID of the differential cell format to apply. */
    std::optional<std::size_t> dxf_id;

    cond_format_date_rule_t();
    cond_format_date_rule_t(const cond_format_date_rule_t& other);
    cond_format_date_rule_t(cond_format_date_rule_t&& other);
    ~cond_format_date_rule_t();

    cond_format_date_rule_t& operator=(cond_format_date_rule_t other);

    void swap(cond_format_date_rule_t& other) noexcept;
};

/**
 * Stores a color scale rule of a conditional format, which colors each cell
 * by interpolating between two or more thresholds.
 */
struct ORCUS_SPM_DLLPUBLIC cond_format_colorscale_rule_t
{
    /** Whether or not to show the value in the cell. */
    bool show_value = true;

    /** Thresholds of the color scale, each with a color. */
    std::vector<cond_format_value_t> values;

    cond_format_colorscale_rule_t();
    cond_format_colorscale_rule_t(const cond_format_colorscale_rule_t& other);
    cond_format_colorscale_rule_t(cond_format_colorscale_rule_t&& other);
    ~cond_format_colorscale_rule_t();

    cond_format_colorscale_rule_t& operator=(cond_format_colorscale_rule_t other);

    void swap(cond_format_colorscale_rule_t& other) noexcept;
};

/**
 * Stores a data bar rule of a conditional format, which draws a bar in each
 * cell whose length reflects the cell value relative to two thresholds.
 */
struct ORCUS_SPM_DLLPUBLIC cond_format_databar_rule_t
{
    /** Whether or not to show the value in the cell. */
    bool show_value = true;

    /** Whether or not to use a gradient for the data bar. */
    bool gradient = true;

    /** Position of the 0 axis of the data bar. */
    databar_axis_t axis = databar_axis_t::none;

    /** Data bar color for positive values. */
    std::optional<color_t> color_positive;

    /** Data bar color for negative values. */
    std::optional<color_t> color_negative;

    /** Minimum length of the data bar. */
    double min_length = 10.0;

    /** Maximum length of the data bar. */
    double max_length = 90.0;

    /** Thresholds of the data bar. */
    std::vector<cond_format_value_t> values;

    cond_format_databar_rule_t();
    cond_format_databar_rule_t(const cond_format_databar_rule_t& other);
    cond_format_databar_rule_t(cond_format_databar_rule_t&& other);
    ~cond_format_databar_rule_t();

    cond_format_databar_rule_t& operator=(cond_format_databar_rule_t other);

    void swap(cond_format_databar_rule_t& other) noexcept;
};

/**
 * Stores an icon set rule of a conditional format, which shows an icon in
 * each cell chosen by comparing the cell value against a series of
 * thresholds.
 */
struct ORCUS_SPM_DLLPUBLIC cond_format_iconset_rule_t
{
    /** Whether or not to show the value in the cell. */
    bool show_value = true;

    /** Set of icons to use. */
    icon_set_t icon_set = icon_set_t::unknown;

    /** Whether or not to use the icons in reverse order. */
    bool reverse = false;

    /** Thresholds of the icon set, one per icon. */
    std::vector<cond_format_value_t> values;

    cond_format_iconset_rule_t();
    cond_format_iconset_rule_t(const cond_format_iconset_rule_t& other);
    cond_format_iconset_rule_t(cond_format_iconset_rule_t&& other);
    ~cond_format_iconset_rule_t();

    cond_format_iconset_rule_t& operator=(cond_format_iconset_rule_t other);

    void swap(cond_format_iconset_rule_t& other) noexcept;
};

/**
 * Stores a single rule of a conditional format, which is one of the five
 * rule types.
 */
using cond_format_rule_t = std::variant<
    cond_format_condition_rule_t,
    cond_format_date_rule_t,
    cond_format_colorscale_rule_t,
    cond_format_databar_rule_t,
    cond_format_iconset_rule_t>;

/**
 * Stores a single conditional format, which consists of one or more cell
 * ranges the format is applied to, and one or more rules.
 */
struct ORCUS_SPM_DLLPUBLIC cond_format_t
{
    /** Cell ranges the conditional format is applied to. */
    std::vector<ixion::abs_rc_range_t> ranges;

    /** Rules of the conditional format. */
    std::vector<cond_format_rule_t> rules;

    cond_format_t();
    cond_format_t(const cond_format_t& other);
    cond_format_t(cond_format_t&& other);
    ~cond_format_t();

    cond_format_t& operator=(cond_format_t other);

    void swap(cond_format_t& other) noexcept;
};

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
