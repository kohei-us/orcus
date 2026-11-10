/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "factory_cond_format.hpp"
#include "formula_global.hpp"

#include <orcus/spreadsheet/sheet.hpp>
#include <orcus/string_pool.hpp>

namespace orcus { namespace spreadsheet {

import_cond_format_value::import_cond_format_value(string_pool& sp, std::vector<cond_format_value_t>& values) :
    m_pool(sp), m_values(values) {}

import_cond_format_value::~import_cond_format_value() = default;

void import_cond_format_value::set_formula(std::string_view formula)
{
    m_value.formula = m_pool.intern(formula).first;
}

void import_cond_format_value::set_type(cond_format_value_type_t type)
{
    m_value.type = type;
}

void import_cond_format_value::set_color(
    color_elem_t alpha, color_elem_t red, color_elem_t green, color_elem_t blue)
{
    m_value.color = color_t{alpha, red, green, blue};
}

void import_cond_format_value::commit()
{
    m_values.push_back(std::move(m_value));
}

void import_cond_format_value::reset()
{
    m_value = cond_format_value_t{};
}

import_cond_format_condition::import_cond_format_condition(string_pool& sp, cond_format_t& format) :
    m_pool(sp), m_format(format), m_import_value(sp, m_rule.values) {}

import_cond_format_condition::~import_cond_format_condition() = default;

void import_cond_format_condition::set_operator(cond_format_operator_t op)
{
    m_rule.op = op;
}

void import_cond_format_condition::set_dxf_id(std::size_t dxf)
{
    m_rule.dxf_id = dxf;
}

void import_cond_format_condition::set_rank(std::string_view rank)
{
    m_rule.rank = m_pool.intern(rank).first;
}

void import_cond_format_condition::set_text(std::string_view text)
{
    m_rule.text = m_pool.intern(text).first;
}

void import_cond_format_condition::set_std_dev(std::string_view sd)
{
    m_rule.std_dev = m_pool.intern(sd).first;
}

iface::import_cond_format_value* import_cond_format_condition::start_value()
{
    m_import_value.reset();
    return &m_import_value;
}

void import_cond_format_condition::commit()
{
    m_format.rules.emplace_back(std::move(m_rule));
}

void import_cond_format_condition::reset()
{
    m_rule = cond_format_condition_rule_t{};
}

import_cond_format_date::import_cond_format_date(cond_format_t& format) : m_format(format) {}
import_cond_format_date::~import_cond_format_date() = default;

void import_cond_format_date::set_period(cond_format_date_t period)
{
    m_rule.period = period;
}

void import_cond_format_date::set_dxf_id(std::size_t dxf)
{
    m_rule.dxf_id = dxf;
}

void import_cond_format_date::commit()
{
    m_format.rules.emplace_back(std::move(m_rule));
}

void import_cond_format_date::reset()
{
    m_rule = cond_format_date_rule_t{};
}

import_cond_format_colorscale::import_cond_format_colorscale(string_pool& sp, cond_format_t& format) :
    m_format(format), m_import_value(sp, m_rule.values) {}

import_cond_format_colorscale::~import_cond_format_colorscale() = default;

void import_cond_format_colorscale::set_show_value(bool show)
{
    m_rule.show_value = show;
}

iface::import_cond_format_value* import_cond_format_colorscale::start_value()
{
    m_import_value.reset();
    return &m_import_value;
}

void import_cond_format_colorscale::commit()
{
    m_format.rules.emplace_back(std::move(m_rule));
}

void import_cond_format_colorscale::reset()
{
    m_rule = cond_format_colorscale_rule_t{};
}

import_cond_format_databar::import_cond_format_databar(string_pool& sp, cond_format_t& format) :
    m_format(format), m_import_value(sp, m_rule.values) {}

import_cond_format_databar::~import_cond_format_databar() = default;

void import_cond_format_databar::set_show_value(bool show)
{
    m_rule.show_value = show;
}

void import_cond_format_databar::set_gradient(bool gradient)
{
    m_rule.gradient = gradient;
}

void import_cond_format_databar::set_axis(databar_axis_t axis)
{
    m_rule.axis = axis;
}

void import_cond_format_databar::set_color_positive(
    color_elem_t alpha, color_elem_t red, color_elem_t green, color_elem_t blue)
{
    m_rule.color_positive = color_t{alpha, red, green, blue};
}

void import_cond_format_databar::set_color_negative(
    color_elem_t alpha, color_elem_t red, color_elem_t green, color_elem_t blue)
{
    m_rule.color_negative = color_t{alpha, red, green, blue};
}

void import_cond_format_databar::set_min_length(double length)
{
    m_rule.min_length = length;
}

void import_cond_format_databar::set_max_length(double length)
{
    m_rule.max_length = length;
}

iface::import_cond_format_value* import_cond_format_databar::start_value()
{
    m_import_value.reset();
    return &m_import_value;
}

void import_cond_format_databar::commit()
{
    m_format.rules.emplace_back(std::move(m_rule));
}

void import_cond_format_databar::reset()
{
    m_rule = cond_format_databar_rule_t{};
}

import_cond_format_iconset::import_cond_format_iconset(string_pool& sp, cond_format_t& format) :
    m_format(format), m_import_value(sp, m_rule.values) {}

import_cond_format_iconset::~import_cond_format_iconset() = default;

void import_cond_format_iconset::set_show_value(bool show)
{
    m_rule.show_value = show;
}

void import_cond_format_iconset::set_icon_set(icon_set_t icons)
{
    m_rule.icon_set = icons;
}

void import_cond_format_iconset::set_reverse(bool reverse)
{
    m_rule.reverse = reverse;
}

iface::import_cond_format_value* import_cond_format_iconset::start_value()
{
    m_import_value.reset();
    return &m_import_value;
}

void import_cond_format_iconset::commit()
{
    m_format.rules.emplace_back(std::move(m_rule));
}

void import_cond_format_iconset::reset()
{
    m_rule = cond_format_iconset_rule_t{};
}

import_cond_format::import_cond_format(string_pool& sp, sheet& sh) :
    m_sheet(sh),
    m_import_condition(sp, m_format),
    m_import_date(m_format),
    m_import_colorscale(sp, m_format),
    m_import_databar(sp, m_format),
    m_import_iconset(sp, m_format) {}

import_cond_format::~import_cond_format() = default;

void import_cond_format::append_range(const range_t& range)
{
    m_format.ranges.push_back(to_abs_rc_range(range));
}

iface::import_cond_format_condition* import_cond_format::start_condition()
{
    m_import_condition.reset();
    return &m_import_condition;
}

iface::import_cond_format_date* import_cond_format::start_date()
{
    m_import_date.reset();
    return &m_import_date;
}

iface::import_cond_format_colorscale* import_cond_format::start_colorscale()
{
    m_import_colorscale.reset();
    return &m_import_colorscale;
}

iface::import_cond_format_databar* import_cond_format::start_databar()
{
    m_import_databar.reset();
    return &m_import_databar;
}

iface::import_cond_format_iconset* import_cond_format::start_iconset()
{
    m_import_iconset.reset();
    return &m_import_iconset;
}

void import_cond_format::commit()
{
    m_sheet.append_cond_format(std::move(m_format));
}

void import_cond_format::reset()
{
    m_format = cond_format_t{};
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
