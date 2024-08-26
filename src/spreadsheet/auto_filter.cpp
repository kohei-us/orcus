/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <orcus/spreadsheet/auto_filter.hpp>

namespace orcus { namespace spreadsheet {

filter_value_t::filter_value_t() = default;
filter_value_t::filter_value_t(double v) : m_store(v) {}
filter_value_t::filter_value_t(std::string_view v) : m_store(v) {}
filter_value_t::filter_value_t(const filter_value_t& other) = default;
filter_value_t::~filter_value_t() = default;

bool filter_value_t::operator==(const filter_value_t& other) const
{
    return m_store == other.m_store;
}

bool filter_value_t::operator!=(const filter_value_t& other) const
{
    return !operator==(other);
}


filter_value_t& filter_value_t::operator=(const filter_value_t& other)
{
    filter_value_t temp(other);
    swap(temp);

    return *this;
}

auto filter_value_t::type() const noexcept -> value_type
{
    switch (m_store.index())
    {
        case 0:
            return value_type::empty;
        case 1:
            return value_type::numeric;
        case 2:
            return value_type::string;
    }

    return value_type::empty; // invalid becomes empty
}

double filter_value_t::numeric() const
{
    return std::get<double>(m_store);
}

std::string_view filter_value_t::string() const
{
    return std::get<std::string_view>(m_store);
}

void filter_value_t::swap(filter_value_t& other) noexcept
{
    std::swap(m_store, other.m_store);
}

filterable::~filterable() = default;

filter_item_t::filter_item_t() : op(auto_filter_op_t::unspecified) {}
filter_item_t::filter_item_t(auto_filter_op_t _op) : op(_op) {}
filter_item_t::filter_item_t(auto_filter_op_t _op, double v) : op(_op), value(v) {}
filter_item_t::filter_item_t(auto_filter_op_t _op, std::string_view v) : op(_op), value(v) {}
filter_item_t::filter_item_t(const filter_item_t& other) = default;
filter_item_t::~filter_item_t() = default;

filter_item_t& filter_item_t::operator=(const filter_item_t& other)
{
    filter_item_t temp(other);
    swap(temp);

    return *this;
}

void filter_item_t::swap(filter_item_t& other) noexcept
{
    std::swap(op, other.op);
    value.swap(other.value);
}

filter_node_t::filter_node_t() : op(auto_filter_node_op_t::unspecified) {}
filter_node_t::filter_node_t(auto_filter_node_op_t _op) : op(_op) {}
filter_node_t::filter_node_t(const filter_node_t& other) :
    op(other.op), children(other.children) {}
filter_node_t::filter_node_t(filter_node_t&& other) :
    op(other.op), children(std::move(other.children)) {}
filter_node_t::~filter_node_t() = default;

filter_node_t& filter_node_t::operator=(const filter_node_t& other)
{
    filter_node_t temp(other);
    swap(temp);

    return *this;
}

filter_node_t& filter_node_t::operator=(filter_node_t&& other)
{
    filter_node_t temp(std::move(other));
    swap(temp);

    return *this;
}

void filter_node_t::reset()
{
    op = auto_filter_node_op_t::unspecified;
    children.clear();
}

void filter_node_t::swap(filter_node_t& other) noexcept
{
    std::swap(op, other.op);
    std::swap(children, other.children);
}

auto_filter_t::auto_filter_t() = default;
auto_filter_t::auto_filter_t(const auto_filter_t& other) = default;
auto_filter_t::auto_filter_t(auto_filter_t&& other) = default;
auto_filter_t::~auto_filter_t() = default;

auto_filter_t& auto_filter_t::operator=(const auto_filter_t& other) = default;
auto_filter_t& auto_filter_t::operator=(auto_filter_t&& other) = default;

void auto_filter_t::reset()
{
    columns.clear();
    node_store.clear();
    item_store.clear();
}

void auto_filter_t::swap(auto_filter_t& other)
{
    columns.swap(other.columns);
    node_store.swap(other.node_store);
    item_store.swap(other.item_store);
}

auto_filter_range_t::auto_filter_range_t() : range(ixion::abs_range_t::invalid) {}
auto_filter_range_t::auto_filter_range_t(const auto_filter_range_t& other) = default;
auto_filter_range_t::auto_filter_range_t(auto_filter_range_t&& other) = default;
auto_filter_range_t::~auto_filter_range_t() = default;

auto_filter_range_t& auto_filter_range_t::operator=(const auto_filter_range_t& other) = default;
auto_filter_range_t& auto_filter_range_t::operator=(auto_filter_range_t&& other) = default;

void auto_filter_range_t::reset()
{
    range = ixion::abs_range_t(ixion::abs_range_t::invalid);
    filter.reset();
}

void auto_filter_range_t::swap(auto_filter_range_t& other)
{
    std::swap(range, other.range);
    filter.swap(other.filter);
}

namespace old {

auto_filter_column_t::auto_filter_column_t() = default;
auto_filter_column_t::auto_filter_column_t(const auto_filter_column_t& other) = default;
auto_filter_column_t::auto_filter_column_t(auto_filter_column_t&& other) = default;
auto_filter_column_t::~auto_filter_column_t() = default;

auto_filter_column_t& auto_filter_column_t::operator=(const auto_filter_column_t& other) = default;
auto_filter_column_t& auto_filter_column_t::operator=(auto_filter_column_t&& other) = default;

void auto_filter_column_t::reset()
{
    match_values.clear();
}

void auto_filter_column_t::swap(auto_filter_column_t& r)
{
    match_values.swap(r.match_values);
}

auto_filter_t::auto_filter_t() : range(ixion::abs_range_t::invalid) {}
auto_filter_t::auto_filter_t(const auto_filter_t& other) = default;
auto_filter_t::auto_filter_t(auto_filter_t&& other) = default;
auto_filter_t::~auto_filter_t() = default;

auto_filter_t& auto_filter_t::operator=(const auto_filter_t& other) = default;
auto_filter_t& auto_filter_t::operator=(auto_filter_t&& other) = default;

void auto_filter_t::reset()
{
    range = ixion::abs_range_t(ixion::abs_range_t::invalid);
    columns.clear();
}

void auto_filter_t::swap(auto_filter_t& r)
{
    std::swap(range, r.range);
    columns.swap(r.columns);
}

void auto_filter_t::commit_column(col_t col, auto_filter_column_t data)
{
    if (col < 0)
        // Invalid column index.  Nothing happens.
        return;

    columns.insert_or_assign(col, std::move(data));
}

} // namespace old

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
