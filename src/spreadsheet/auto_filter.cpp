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

bool filter_value_t::operator<(const filter_value_t& other) const
{
    auto index = m_store.index();
    if (index != other.m_store.index())
        return index < other.m_store.index();

    switch (index)
    {
        case 0:
            return true;
        case 1:
            return numeric() < other.numeric();
        case 2:
            return string() < other.string();
    }

    return false; // invalid value type
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

filter_item_t::filter_item_t() : m_op(auto_filter_op_t::unspecified) {}
filter_item_t::filter_item_t(col_t _field, auto_filter_op_t _op) :
    m_field(_field), m_op(_op) {}
filter_item_t::filter_item_t(col_t _field, auto_filter_op_t _op, double v) :
    m_field(_field), m_op(_op), m_value(v) {}
filter_item_t::filter_item_t(col_t _field, auto_filter_op_t _op, std::string_view v) :
    m_field(_field), m_op(_op), m_value(v) {}
filter_item_t::filter_item_t(col_t _field, auto_filter_op_t _op, std::string_view v, bool _regex) :
    m_field(_field), m_op(_op), m_value(v), m_regex(_regex) {}
filter_item_t::filter_item_t(const filter_item_t& other) = default;
filter_item_t::~filter_item_t() = default;

col_t filter_item_t::field() const
{
    return m_field;
}

auto_filter_op_t filter_item_t::op() const
{
    return m_op;
}

filter_value_t filter_item_t::value() const
{
    return m_value;
}

bool filter_item_t::regex() const
{
    return m_regex;
}

filter_item_t& filter_item_t::operator=(const filter_item_t& other)
{
    filter_item_t temp(other);
    swap(temp);

    return *this;
}

void filter_item_t::swap(filter_item_t& other) noexcept
{
    std::swap(m_field, other.m_field);
    std::swap(m_op, other.m_op);
    std::swap(m_regex, other.m_regex);
    m_value.swap(other.m_value);
}

bool filter_item_t::operator==(const filter_item_t& other) const
{
    if (m_field != other.m_field)
        return false;

    if (m_op != other.m_op)
        return false;

    if (m_regex != other.m_regex)
        return false;

    return m_value == other.m_value;
}

bool filter_item_t::operator!=(const filter_item_t& other) const
{
    return !operator==(other);
}

bool filter_item_t::operator<(const filter_item_t& other) const
{
    if (m_field != other.m_field)
        return m_field < other.m_field;

    using utype = std::underlying_type<auto_filter_op_t>::type;

    if (m_op != other.m_op)
        return static_cast<utype>(m_op) < static_cast<utype>(other.m_op);

    if (m_regex != other.m_regex)
        return m_regex < other.m_regex;

    return m_value < other.m_value;
}

filter_item_set_t::filter_item_set_t() = default;
filter_item_set_t::filter_item_set_t(col_t _field) : m_field(_field) {}
filter_item_set_t::filter_item_set_t(col_t field, std::initializer_list<std::string_view> values) :
    m_field(field), m_values(values) {}

filter_item_set_t::filter_item_set_t(const filter_item_set_t& other) = default;
filter_item_set_t::filter_item_set_t(filter_item_set_t&& other) = default;
filter_item_set_t::~filter_item_set_t() = default;

const std::unordered_set<std::string_view>& filter_item_set_t::values() const
{
    return m_values;
}

col_t filter_item_set_t::field() const
{
    return m_field;
}

void filter_item_set_t::insert(std::string_view value)
{
    m_values.insert(value);
}

filter_item_set_t& filter_item_set_t::operator=(const filter_item_set_t& other)
{
    filter_item_set_t temp(other);
    swap(temp);

    return *this;
}
filter_item_set_t& filter_item_set_t::operator=(filter_item_set_t&& other)
{
    filter_item_set_t temp(std::move(other));
    swap(temp);

    return *this;
}

void filter_item_set_t::reset()
{
    m_field = -1;
    m_values.clear();
}

void filter_item_set_t::swap(filter_item_set_t& other) noexcept
{
    std::swap(m_field, other.m_field);
    m_values.swap(other.m_values);
}

bool filter_item_set_t::operator==(const filter_item_set_t& other) const
{
    if (m_field != other.m_field)
        return false;

    return m_values == other.m_values;
}

bool filter_item_set_t::operator!=(const filter_item_set_t& other) const
{
    return !operator==(other);
}

struct filter_node_t::impl
{
    using node_store_type = std::deque<filter_node_t>;
    using item_store_type = std::deque<filter_item_t>;
    using item_set_store_type = std::deque<filter_item_set_t>;

    auto_filter_node_op_t op;
    filter_node_t::children_type children;
    node_store_type node_store;
    item_store_type item_store;
    item_set_store_type item_set_store;

    impl() : op(auto_filter_node_op_t::unspecified) {}
    impl(auto_filter_node_op_t _op) : op(_op) {}
};

filter_node_t::filter_node_t() : m_impl(std::make_unique<impl>()) {}
filter_node_t::filter_node_t(auto_filter_node_op_t _op) : m_impl(std::make_unique<impl>(_op)) {}
filter_node_t::filter_node_t(filter_node_t&& other) = default;
filter_node_t::~filter_node_t() = default;

filter_node_t& filter_node_t::operator=(filter_node_t&& other)
{
    filter_node_t temp(std::move(other));
    swap(temp);

    return *this;
}

auto_filter_node_op_t filter_node_t::op() const
{
    return m_impl->op;
}

auto filter_node_t::children() const -> const children_type&
{
    return m_impl->children;
}

void filter_node_t::append(filter_node_t child)
{
    m_impl->node_store.push_back(std::move(child));
    m_impl->children.push_back(&m_impl->node_store.back());
}

void filter_node_t::append(filter_item_t child)
{
    m_impl->item_store.push_back(std::move(child));
    m_impl->children.push_back(&m_impl->item_store.back());
}

void filter_node_t::append(filter_item_set_t child)
{
    m_impl->item_set_store.push_back(std::move(child));
    m_impl->children.push_back(&m_impl->item_set_store.back());
}

void filter_node_t::reset()
{
    m_impl = std::make_unique<impl>();
}

void filter_node_t::swap(filter_node_t& other) noexcept
{
    std::swap(m_impl, other.m_impl);
}

auto_filter_t::auto_filter_t() = default;
auto_filter_t::auto_filter_t(auto_filter_t&& other) = default;
auto_filter_t::~auto_filter_t() = default;

auto_filter_t& auto_filter_t::operator=(auto_filter_t&& other) = default;

void auto_filter_t::reset()
{
    range = ixion::abs_rc_range_t(ixion::abs_range_t::invalid);
    root.reset();
}

void auto_filter_t::swap(auto_filter_t& other)
{
    std::swap(range, other.range);
    root.swap(other.root);
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
