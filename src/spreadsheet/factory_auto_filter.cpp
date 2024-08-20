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

namespace orcus { namespace spreadsheet {

void import_auto_filter_node::append_item(auto_filter_op_t op, std::string_view value)
{
    (void)op;
    (void)value;
}

void import_auto_filter_node::append_item(auto_filter_op_t op, double value)
{
    (void)op;
    (void)value;
}

void import_auto_filter_node::commit() {}

import_auto_filter::import_auto_filter() {}
import_auto_filter::~import_auto_filter() = default;

iface::import_auto_filter_node* import_auto_filter::start_column(col_t col, auto_filter_node_op_t op)
{
    (void)col;
    (void)op;

    return nullptr;
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
