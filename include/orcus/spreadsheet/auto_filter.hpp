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

struct ORCUS_SPM_DLLPUBLIC filterable
{
    virtual ~filterable();
};

/**
 * Represents a single filtering criterion for a field.
 */
struct ORCUS_SPM_DLLPUBLIC filter_item_t : filterable
{
    col_t field = -1;
    auto_filter_op_t op = auto_filter_op_t::unspecified;
    filter_value_t value;

    filter_item_t();
    filter_item_t(col_t _field, auto_filter_op_t _op);
    filter_item_t(col_t _field, auto_filter_op_t _op, double v);
    filter_item_t(col_t _field, auto_filter_op_t _op, std::string_view v);
    filter_item_t(const filter_item_t& other);
    ~filter_item_t() override;

    filter_item_t& operator=(const filter_item_t& other);

    void swap(filter_item_t& other) noexcept;

    bool operator==(const filter_item_t& other) const;
    bool operator!=(const filter_item_t& other) const;
    bool operator<(const filter_item_t& other) const;
};

/**
 * Represents a single node in a boolean tree of filtering criteria connected
 * with boolean operators.
 */
struct ORCUS_SPM_DLLPUBLIC filter_node_t : filterable
{
    using children_type = std::deque<filterable*>;
    using node_store_type = std::deque<filter_node_t>;
    using item_store_type = std::deque<filter_item_t>;

    auto_filter_node_op_t op;
    children_type children;
    node_store_type node_store;
    item_store_type item_store;

    filter_node_t();
    filter_node_t(auto_filter_node_op_t _op);

    filter_node_t(const filter_node_t& other);
    filter_node_t(filter_node_t&& other);
    ~filter_node_t() override;

    filter_node_t& operator=(const filter_node_t& other);
    filter_node_t& operator=(filter_node_t&& other);

    void reset();
    void swap(filter_node_t& other) noexcept;
};

/**
 * Data for a single auto-filter entry.  An auto-filter can belong to either a
 * sheet or a table.
 */
struct ORCUS_SPM_DLLPUBLIC auto_filter_t
{
    filter_node_t root;

    auto_filter_t();
    auto_filter_t(const auto_filter_t& other);
    auto_filter_t(auto_filter_t&& other);
    ~auto_filter_t();

    auto_filter_t& operator=(const auto_filter_t& other);
    auto_filter_t& operator=(auto_filter_t&& other);

    void reset();
    void swap(auto_filter_t& other);
};

struct ORCUS_SPM_DLLPUBLIC auto_filter_range_t
{
    ixion::abs_rc_range_t range;
    auto_filter_t filter;

    auto_filter_range_t();
    auto_filter_range_t(const auto_filter_range_t& other);
    auto_filter_range_t(auto_filter_range_t&& other);
    ~auto_filter_range_t();

    auto_filter_range_t& operator=(const auto_filter_range_t& other);
    auto_filter_range_t& operator=(auto_filter_range_t&& other);

    void reset();
    void swap(auto_filter_range_t& other);
};

namespace old {

/**
 * Data for a single column inside autofilter range.
 */
struct ORCUS_SPM_DLLPUBLIC auto_filter_column_t
{
    using match_values_type = std::unordered_set<std::string_view>;
    match_values_type match_values;

    auto_filter_column_t();
    auto_filter_column_t(const auto_filter_column_t& other);
    auto_filter_column_t(auto_filter_column_t&& other);
    ~auto_filter_column_t();

    auto_filter_column_t& operator=(const auto_filter_column_t& other);
    auto_filter_column_t& operator=(auto_filter_column_t&& other);

    void reset();
    void swap(auto_filter_column_t& r);
};

/**
 * Data for a single autofilter entry.  An autofilter can belong to either a
 * sheet or a table.
 */
struct ORCUS_SPM_DLLPUBLIC auto_filter_t
{
    typedef std::map<col_t, auto_filter_column_t> columns_type;

    ixion::abs_range_t range;

    columns_type columns;

    auto_filter_t();
    auto_filter_t(const auto_filter_t& other);
    auto_filter_t(auto_filter_t&& other);
    ~auto_filter_t();

    auto_filter_t& operator=(const auto_filter_t& other);
    auto_filter_t& operator=(auto_filter_t&& other);

    void reset();
    void swap(auto_filter_t& r);

    /**
     * Set column data to specified column index.
     *
     * @param col column index to associate the data to.
     * @param data column data.
     */
    void commit_column(col_t col, auto_filter_column_t data);
};

} // namespace old

}}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
