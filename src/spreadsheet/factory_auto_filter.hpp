/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <orcus/spreadsheet/import_interface_auto_filter.hpp>
#include <orcus/spreadsheet/auto_filter.hpp>

#include <memory>
#include <functional>

namespace orcus {

class string_pool;

namespace spreadsheet {

class sheet;

class import_auto_filter_multi_values : public orcus::spreadsheet::iface::import_auto_filter_multi_values
{
public:
    using commit_func_type = std::function<void(filter_item_set_t&&)>;

    import_auto_filter_multi_values(string_pool& sp);
    import_auto_filter_multi_values(const import_auto_filter_multi_values&) = delete;
    ~import_auto_filter_multi_values();

    virtual void add_value(std::string_view value) override;
    virtual void commit() override;

    void reset(col_t field, commit_func_type func);

private:
    string_pool& m_pool;

    filter_item_set_t m_set;
    commit_func_type m_func_commit;
};

class import_auto_filter_node : public orcus::spreadsheet::iface::import_auto_filter_node
{
public:
    using commit_func_type = std::function<void(filter_node_t&&)>;

    import_auto_filter_node(string_pool& sp);
    import_auto_filter_node(const import_auto_filter_node&) = delete;
    import_auto_filter_node(string_pool& sp, auto_filter_node_op_t op, commit_func_type func);
    ~import_auto_filter_node();

    virtual void append_item(col_t field, auto_filter_op_t op, std::string_view value, bool regex) override;
    virtual void append_item(col_t field, auto_filter_op_t op, double value) override;
    virtual iface::import_auto_filter_node* start_node(auto_filter_node_op_t op) override;
    virtual iface::import_auto_filter_multi_values* start_multi_values(col_t field) override;
    virtual void commit() override;

    void reset(auto_filter_node_op_t op, commit_func_type func);

private:
    string_pool& m_pool;

    filter_node_t m_node;
    commit_func_type m_func_commit;

    import_auto_filter_multi_values m_import_multi_values;
    std::unique_ptr<import_auto_filter_node> mp_child;
};

class import_auto_filter : public orcus::spreadsheet::iface::import_auto_filter
{
public:
    using commit_func_type = std::function<void(auto_filter_t&&)>;

    import_auto_filter(string_pool& sp);
    import_auto_filter(const import_auto_filter&) = delete;
    ~import_auto_filter();

    virtual iface::import_auto_filter_node* start_node(auto_filter_node_op_t op) override;
    virtual void commit() override;

    void reset(commit_func_type func, const ixion::abs_range_t& range);

private:
    string_pool& m_pool;
    import_auto_filter_node m_import_root_node;

    auto_filter_t m_filter;
    commit_func_type m_func_commit;
};

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
