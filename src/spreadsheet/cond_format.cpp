/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "orcus/spreadsheet/cond_format.hpp"

#include <utility>

namespace orcus { namespace spreadsheet {

cond_format_value_t::cond_format_value_t() = default;
cond_format_value_t::cond_format_value_t(const cond_format_value_t& other) = default;
cond_format_value_t::cond_format_value_t(cond_format_value_t&& other) = default;
cond_format_value_t::~cond_format_value_t() = default;

cond_format_value_t& cond_format_value_t::operator=(cond_format_value_t other)
{
    swap(other);
    return *this;
}

void cond_format_value_t::swap(cond_format_value_t& other) noexcept
{
    std::swap(formula, other.formula);
    std::swap(type, other.type);
    std::swap(color, other.color);
}

cond_format_condition_rule_t::cond_format_condition_rule_t() = default;
cond_format_condition_rule_t::cond_format_condition_rule_t(const cond_format_condition_rule_t& other) = default;
cond_format_condition_rule_t::cond_format_condition_rule_t(cond_format_condition_rule_t&& other) = default;
cond_format_condition_rule_t::~cond_format_condition_rule_t() = default;

cond_format_condition_rule_t& cond_format_condition_rule_t::operator=(cond_format_condition_rule_t other)
{
    swap(other);
    return *this;
}

void cond_format_condition_rule_t::swap(cond_format_condition_rule_t& other) noexcept
{
    std::swap(op, other.op);
    std::swap(dxf_id, other.dxf_id);
    std::swap(rank, other.rank);
    std::swap(text, other.text);
    std::swap(std_dev, other.std_dev);
    std::swap(values, other.values);
}

cond_format_date_rule_t::cond_format_date_rule_t() = default;
cond_format_date_rule_t::cond_format_date_rule_t(const cond_format_date_rule_t& other) = default;
cond_format_date_rule_t::cond_format_date_rule_t(cond_format_date_rule_t&& other) = default;
cond_format_date_rule_t::~cond_format_date_rule_t() = default;

cond_format_date_rule_t& cond_format_date_rule_t::operator=(cond_format_date_rule_t other)
{
    swap(other);
    return *this;
}

void cond_format_date_rule_t::swap(cond_format_date_rule_t& other) noexcept
{
    std::swap(period, other.period);
    std::swap(dxf_id, other.dxf_id);
}

cond_format_colorscale_rule_t::cond_format_colorscale_rule_t() = default;
cond_format_colorscale_rule_t::cond_format_colorscale_rule_t(const cond_format_colorscale_rule_t& other) = default;
cond_format_colorscale_rule_t::cond_format_colorscale_rule_t(cond_format_colorscale_rule_t&& other) = default;
cond_format_colorscale_rule_t::~cond_format_colorscale_rule_t() = default;

cond_format_colorscale_rule_t& cond_format_colorscale_rule_t::operator=(cond_format_colorscale_rule_t other)
{
    swap(other);
    return *this;
}

void cond_format_colorscale_rule_t::swap(cond_format_colorscale_rule_t& other) noexcept
{
    std::swap(values, other.values);
}

cond_format_databar_rule_t::cond_format_databar_rule_t() = default;
cond_format_databar_rule_t::cond_format_databar_rule_t(const cond_format_databar_rule_t& other) = default;
cond_format_databar_rule_t::cond_format_databar_rule_t(cond_format_databar_rule_t&& other) = default;
cond_format_databar_rule_t::~cond_format_databar_rule_t() = default;

cond_format_databar_rule_t& cond_format_databar_rule_t::operator=(cond_format_databar_rule_t other)
{
    swap(other);
    return *this;
}

void cond_format_databar_rule_t::swap(cond_format_databar_rule_t& other) noexcept
{
    std::swap(show_value, other.show_value);
    std::swap(gradient, other.gradient);
    std::swap(axis, other.axis);
    std::swap(color_positive, other.color_positive);
    std::swap(color_negative, other.color_negative);
    std::swap(min_length, other.min_length);
    std::swap(max_length, other.max_length);
    std::swap(values, other.values);
}

cond_format_iconset_rule_t::cond_format_iconset_rule_t() = default;
cond_format_iconset_rule_t::cond_format_iconset_rule_t(const cond_format_iconset_rule_t& other) = default;
cond_format_iconset_rule_t::cond_format_iconset_rule_t(cond_format_iconset_rule_t&& other) = default;
cond_format_iconset_rule_t::~cond_format_iconset_rule_t() = default;

cond_format_iconset_rule_t& cond_format_iconset_rule_t::operator=(cond_format_iconset_rule_t other)
{
    swap(other);
    return *this;
}

void cond_format_iconset_rule_t::swap(cond_format_iconset_rule_t& other) noexcept
{
    std::swap(show_value, other.show_value);
    std::swap(icon_set, other.icon_set);
    std::swap(reverse, other.reverse);
    std::swap(values, other.values);
}

cond_format_t::cond_format_t() = default;
cond_format_t::cond_format_t(const cond_format_t& other) = default;
cond_format_t::cond_format_t(cond_format_t&& other) = default;
cond_format_t::~cond_format_t() = default;

cond_format_t& cond_format_t::operator=(cond_format_t other)
{
    swap(other);
    return *this;
}

void cond_format_t::swap(cond_format_t& other) noexcept
{
    std::swap(ranges, other.ranges);
    std::swap(rules, other.rules);
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
