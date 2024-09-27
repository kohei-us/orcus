/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "factory_table.hpp"
#include "factory_auto_filter.hpp"
#include "formula_global.hpp"

#include <orcus/string_pool.hpp>
#include <orcus/spreadsheet/document.hpp>
#include <orcus/spreadsheet/sheet.hpp>
#include <orcus/spreadsheet/table.hpp>
#include <orcus/spreadsheet/import_interface_auto_filter.hpp>

#include <ixion/formula_name_resolver.hpp>

namespace orcus { namespace spreadsheet {

struct import_table::impl
{
    document& doc;
    sheet& sh;

    import_auto_filter auto_filter;

    std::unique_ptr<table_t> table;
    table_column_t column;

    impl(const impl&) = delete;
    impl& operator=(const impl&) = delete;

    impl(document& _doc, sheet& _sh) :
        doc(_doc), sh(_sh), auto_filter(doc.get_string_pool()) {}
};

import_table::import_table(document& doc, sheet& sh) : mp_impl(std::make_unique<impl>(doc, sh)) {}
import_table::~import_table() = default;

iface::import_auto_filter* import_table::start_auto_filter(const range_t& range)
{
    auto_filter_t& dest = mp_impl->table->filter;
    import_auto_filter::commit_func_type func = [&dest](auto_filter_t&& filter)
    {
        dest.swap(filter);
    };

    mp_impl->auto_filter.reset(std::move(func), to_abs_range(range, mp_impl->sh.get_index()));
    return &mp_impl->auto_filter;
}

void import_table::set_range(const range_t& range)
{
    mp_impl->table->range = to_abs_range(range, mp_impl->sh.get_index());
}

void import_table::set_identifier(size_t id)
{
    mp_impl->table->identifier = id;
}

void import_table::set_name(std::string_view name)
{
    string_pool& sp = mp_impl->doc.get_string_pool();
    mp_impl->table->name = sp.intern(name).first;
}

void import_table::set_display_name(std::string_view name)
{
    string_pool& sp = mp_impl->doc.get_string_pool();
    mp_impl->table->display_name = sp.intern(name).first;
}

void import_table::set_totals_row_count(size_t row_count)
{
    mp_impl->table->totals_row_count = row_count;
}

void import_table::set_column_count(size_t n)
{
    mp_impl->table->columns.reserve(n);
}

void import_table::set_column_identifier(size_t id)
{
    mp_impl->column.identifier = id;
}

void import_table::set_column_name(std::string_view name)
{
    string_pool& sp = mp_impl->doc.get_string_pool();
    mp_impl->column.name = sp.intern(name).first;
}

void import_table::set_column_totals_row_label(std::string_view label)
{
    string_pool& sp = mp_impl->doc.get_string_pool();
    mp_impl->column.totals_row_label = sp.intern(label).first;
}

void import_table::set_column_totals_row_function(orcus::spreadsheet::totals_row_function_t func)
{
    mp_impl->column.totals_row_function = func;
}

void import_table::commit_column()
{
    mp_impl->table->columns.push_back(mp_impl->column);
    mp_impl->column.reset();
}

void import_table::set_style_name(std::string_view name)
{
    table_style_t& style = mp_impl->table->style;
    string_pool& sp = mp_impl->doc.get_string_pool();
    style.name = sp.intern(name).first;
}

void import_table::set_style_show_first_column(bool b)
{
    table_style_t& style = mp_impl->table->style;
    style.show_first_column = b;
}

void import_table::set_style_show_last_column(bool b)
{
    table_style_t& style = mp_impl->table->style;
    style.show_last_column = b;
}

void import_table::set_style_show_row_stripes(bool b)
{
    table_style_t& style = mp_impl->table->style;
    style.show_row_stripes = b;
}

void import_table::set_style_show_column_stripes(bool b)
{
    table_style_t& style = mp_impl->table->style;
    style.show_column_stripes = b;
}

void import_table::commit()
{
    mp_impl->doc.insert_table(std::move(mp_impl->table));
}

void import_table::reset()
{
    mp_impl->table = std::make_unique<table_t>();
    mp_impl->column.reset();
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
