/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <orcus/spreadsheet/import_interface_cond_format.hpp>
#include <orcus/spreadsheet/cond_format.hpp>

namespace orcus {

class string_pool;

namespace spreadsheet {

class sheet;

class import_cond_format_value : public iface::import_cond_format_value
{
public:
    import_cond_format_value(string_pool& sp, std::vector<cond_format_value_t>& values);
    import_cond_format_value(const import_cond_format_value&) = delete;
    ~import_cond_format_value();

    virtual void set_formula(std::string_view formula) override;
    virtual void set_type(cond_format_value_type_t type) override;
    virtual void set_color(
        color_elem_t alpha, color_elem_t red, color_elem_t green, color_elem_t blue) override;
    virtual void commit() override;

    void reset();

private:
    string_pool& m_pool;
    std::vector<cond_format_value_t>& m_values;

    cond_format_value_t m_value;
};

class import_cond_format_condition : public iface::import_cond_format_condition
{
public:
    import_cond_format_condition(string_pool& sp, cond_format_t& format);
    import_cond_format_condition(const import_cond_format_condition&) = delete;
    ~import_cond_format_condition();

    virtual void set_operator(cond_format_operator_t op) override;
    virtual void set_dxf_id(std::size_t dxf) override;
    virtual void set_rank(std::string_view rank) override;
    virtual void set_text(std::string_view text) override;
    virtual void set_std_dev(std::string_view sd) override;
    virtual iface::import_cond_format_value* start_value() override;
    virtual void commit() override;

    void reset();

private:
    string_pool& m_pool;
    cond_format_t& m_format;

    cond_format_condition_rule_t m_rule;
    import_cond_format_value m_import_value;
};

class import_cond_format_date : public iface::import_cond_format_date
{
public:
    import_cond_format_date(cond_format_t& format);
    import_cond_format_date(const import_cond_format_date&) = delete;
    ~import_cond_format_date();

    virtual void set_period(cond_format_date_t period) override;
    virtual void set_dxf_id(std::size_t dxf) override;
    virtual void commit() override;

    void reset();

private:
    cond_format_t& m_format;

    cond_format_date_rule_t m_rule;
};

class import_cond_format_colorscale : public iface::import_cond_format_colorscale
{
public:
    import_cond_format_colorscale(string_pool& sp, cond_format_t& format);
    import_cond_format_colorscale(const import_cond_format_colorscale&) = delete;
    ~import_cond_format_colorscale();

    virtual void set_show_value(bool show) override;
    virtual iface::import_cond_format_value* start_value() override;
    virtual void commit() override;

    void reset();

private:
    cond_format_t& m_format;

    cond_format_colorscale_rule_t m_rule;
    import_cond_format_value m_import_value;
};

class import_cond_format_databar : public iface::import_cond_format_databar
{
public:
    import_cond_format_databar(string_pool& sp, cond_format_t& format);
    import_cond_format_databar(const import_cond_format_databar&) = delete;
    ~import_cond_format_databar();

    virtual void set_show_value(bool show) override;
    virtual void set_gradient(bool gradient) override;
    virtual void set_axis(databar_axis_t axis) override;
    virtual void set_color_positive(
        color_elem_t alpha, color_elem_t red, color_elem_t green, color_elem_t blue) override;
    virtual void set_color_negative(
        color_elem_t alpha, color_elem_t red, color_elem_t green, color_elem_t blue) override;
    virtual void set_min_length(double length) override;
    virtual void set_max_length(double length) override;
    virtual iface::import_cond_format_value* start_value() override;
    virtual void commit() override;

    void reset();

private:
    cond_format_t& m_format;

    cond_format_databar_rule_t m_rule;
    import_cond_format_value m_import_value;
};

class import_cond_format_iconset : public iface::import_cond_format_iconset
{
public:
    import_cond_format_iconset(string_pool& sp, cond_format_t& format);
    import_cond_format_iconset(const import_cond_format_iconset&) = delete;
    ~import_cond_format_iconset();

    virtual void set_show_value(bool show) override;
    virtual void set_icon_set(icon_set_t icons) override;
    virtual void set_reverse(bool reverse) override;
    virtual iface::import_cond_format_value* start_value() override;
    virtual void commit() override;

    void reset();

private:
    cond_format_t& m_format;

    cond_format_iconset_rule_t m_rule;
    import_cond_format_value m_import_value;
};

class import_cond_format : public iface::import_cond_format
{
public:
    import_cond_format(string_pool& sp, sheet& sh);
    import_cond_format(const import_cond_format&) = delete;
    ~import_cond_format();

    virtual void append_range(const range_t& range) override;
    virtual iface::import_cond_format_condition* start_condition() override;
    virtual iface::import_cond_format_date* start_date() override;
    virtual iface::import_cond_format_colorscale* start_colorscale() override;
    virtual iface::import_cond_format_databar* start_databar() override;
    virtual iface::import_cond_format_iconset* start_iconset() override;
    virtual void commit() override;

    void reset();

private:
    sheet& m_sheet;

    cond_format_t m_format;

    import_cond_format_condition m_import_condition;
    import_cond_format_date m_import_date;
    import_cond_format_colorscale m_import_colorscale;
    import_cond_format_databar m_import_databar;
    import_cond_format_iconset m_import_iconset;
};

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
