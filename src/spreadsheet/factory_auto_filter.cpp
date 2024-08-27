/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "factory_auto_filter.hpp"
#include "formula_global.hpp"

#include <orcus/spreadsheet/auto_filter.hpp>
#include <orcus/spreadsheet/sheet.hpp>
#include <orcus/string_pool.hpp>

namespace orcus { namespace spreadsheet {

import_auto_filter_node::import_auto_filter_node(string_pool& sp) : m_pool(sp) {}

import_auto_filter_node::import_auto_filter_node(string_pool& sp, auto_filter_node_op_t op, commit_func_type func) :
    m_pool(sp), m_node(op), m_func_commit(std::move(func)) {}

import_auto_filter_node::~import_auto_filter_node() = default;

void import_auto_filter_node::append_item(auto_filter_op_t op, std::string_view value)
{
    auto interned = m_pool.intern(value).first;
    m_node.item_store.emplace_back(op, interned);
    m_node.children.push_back(&m_node.item_store.back());
}

void import_auto_filter_node::append_item(auto_filter_op_t op, double value)
{
    m_node.item_store.emplace_back(op, value);
    m_node.children.push_back(&m_node.item_store.back());
}

import_auto_filter_node* import_auto_filter_node::start_node(auto_filter_node_op_t op)
{
    commit_func_type func = [this](filter_node_t&& node)
    {
        m_node.node_store.push_back(std::move(node));
        m_node.children.push_back(&m_node.node_store.back());
    };

    mp_child = std::make_unique<import_auto_filter_node>(m_pool, op, std::move(func));
    return mp_child.get();
}

void import_auto_filter_node::commit()
{
    m_func_commit(std::move(m_node));
}

void import_auto_filter_node::reset(auto_filter_node_op_t op, commit_func_type func)
{
    m_node = filter_node_t{op};
    m_func_commit = std::move(func);
}

import_auto_filter::import_auto_filter(string_pool& sp) : m_pool(sp), m_import_column_node(sp) {}
import_auto_filter::~import_auto_filter() = default;

iface::import_auto_filter_node* import_auto_filter::start_column(col_t col_offset, auto_filter_node_op_t op)
{
    auto& filter = m_filter;

    import_auto_filter_node::commit_func_type func = [&filter, col_offset](filter_node_t&& node)
    {
        filter.columns.insert_or_assign(col_offset, std::move(node));
    };

    m_import_column_node.reset(op, std::move(func));
    return &m_import_column_node;
}

void import_auto_filter::commit()
{
    m_func_commit(std::move(m_filter));
}

void import_auto_filter::reset(commit_func_type func)
{
    m_filter = auto_filter_t{};
    m_func_commit = std::move(func);
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
