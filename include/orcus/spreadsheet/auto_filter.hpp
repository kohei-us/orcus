/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_ORCUS_SPREADSHEET_AUTO_FILTER_HPP
#define INCLUDED_ORCUS_SPREADSHEET_AUTO_FILTER_HPP

#include "types.hpp"
#include "../env.hpp"

#include <map>
#include <deque>
#include <memory>
#include <variant>
#include <unordered_set>
#include <unordered_map>

#include <ixion/address.hpp>

namespace orcus { namespace spreadsheet {

/**
 * Represents a single value associated with a filtering criterion.
 * It can store either a numeric value, a string value, or an empty state.
 */
class ORCUS_SPM_DLLPUBLIC filter_value_t
{
    using store_type = std::variant<std::monostate, double, std::string_view>;
    store_type m_store;

public:
    enum class value_type { empty, numeric, string };

    filter_value_t();
    filter_value_t(double v);
    filter_value_t(std::string_view v);
    filter_value_t(const filter_value_t& other);
    ~filter_value_t();

    bool operator==(const filter_value_t& other) const;
    bool operator!=(const filter_value_t& other) const;
    bool operator<(const filter_value_t& other) const;

    filter_value_t& operator=(const filter_value_t& other);

    value_type type() const noexcept;

    double numeric() const;

    std::string_view string() const;

    void swap(filter_value_t& other) noexcept;
};

class ORCUS_SPM_DLLPUBLIC filterable
{
public:
    virtual ~filterable();
};

/**
 * Represents a single filtering criterion for a field.
 */
class ORCUS_SPM_DLLPUBLIC filter_item_t : public filterable
{
    col_t m_field = -1;
    auto_filter_op_t m_op = auto_filter_op_t::unspecified;
    filter_value_t m_value;
    bool m_regex = false;

public:
    filter_item_t();
    filter_item_t(col_t _field, auto_filter_op_t _op);
    filter_item_t(col_t _field, auto_filter_op_t _op, double v);
    filter_item_t(col_t _field, auto_filter_op_t _op, std::string_view v);
    filter_item_t(col_t _field, auto_filter_op_t _op, std::string_view v, bool _regex);
    filter_item_t(const filter_item_t& other);
    ~filter_item_t() override;

    col_t field() const;
    auto_filter_op_t op() const;
    filter_value_t value() const;
    bool regex() const;

    filter_item_t& operator=(const filter_item_t& other);

    void swap(filter_item_t& other) noexcept;

    bool operator==(const filter_item_t& other) const;
    bool operator!=(const filter_item_t& other) const;
    bool operator<(const filter_item_t& other) const;
};

class ORCUS_SPM_DLLPUBLIC filter_item_set_t : public filterable
{
    col_t m_field = -1;
    std::unordered_set<std::string_view> m_values;

public:
    filter_item_set_t();
    filter_item_set_t(col_t _field);
    filter_item_set_t(col_t field, std::initializer_list<std::string_view> values);
    filter_item_set_t(const filter_item_set_t& other);
    filter_item_set_t(filter_item_set_t&& other);
    ~filter_item_set_t() override;

    col_t field() const;
    const std::unordered_set<std::string_view>& values() const;
    void insert(std::string_view value);

    filter_item_set_t& operator=(const filter_item_set_t& other);
    filter_item_set_t& operator=(filter_item_set_t&& other);

    void reset();
    void swap(filter_item_set_t& other) noexcept;

    bool operator==(const filter_item_set_t& other) const;
    bool operator!=(const filter_item_set_t& other) const;
};

/**
 * Represents a single node in a boolean tree of filtering criteria connected
 * with boolean operators.
 */
class ORCUS_SPM_DLLPUBLIC filter_node_t : public filterable
{
    struct impl;
    std::unique_ptr<impl> m_impl;

public:
    filter_node_t();
    filter_node_t(auto_filter_node_op_t _op);

    filter_node_t(const filter_node_t& other) = delete;
    filter_node_t(filter_node_t&& other);
    ~filter_node_t() override;

    filter_node_t& operator=(const filter_node_t& other) = delete;
    filter_node_t& operator=(filter_node_t&& other);

    auto_filter_node_op_t op() const noexcept;

    /**
     * Returns the number of child filterables.
     */
    std::size_t size() const noexcept;

    bool empty() const noexcept;

    /**
     * Returns the pointer to a child filterable at specified position.
     */
    const filterable* at(std::size_t pos) const;

    void append(filter_node_t child);
    void append(filter_item_t child);
    void append(filter_item_set_t child);

    void reset();
    void swap(filter_node_t& other) noexcept;
};

/**
 * Data for a single auto-filter entry.  An auto-filter can belong to either a
 * sheet or a table.
 */
struct ORCUS_SPM_DLLPUBLIC auto_filter_t
{
    ixion::abs_rc_range_t range;
    filter_node_t root;

    auto_filter_t();
    auto_filter_t(const auto_filter_t& other) = delete;
    auto_filter_t(auto_filter_t&& other);
    ~auto_filter_t();

    auto_filter_t& operator=(const auto_filter_t& other) = delete;
    auto_filter_t& operator=(auto_filter_t&& other);

    void reset();
    void swap(auto_filter_t& other);
};

ORCUS_SPM_DLLPUBLIC std::ostream& operator<<(std::ostream& os, const filter_item_t& v);

}}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
