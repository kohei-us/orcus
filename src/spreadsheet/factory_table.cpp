/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "factory_table.hpp"
#include "formula_global.hpp"

#include "orcus/global.hpp"
#include "orcus/string_pool.hpp"
#include "orcus/spreadsheet/document.hpp"
#include "orcus/spreadsheet/sheet.hpp"
#include "orcus/spreadsheet/auto_filter.hpp"

#include <ixion/formula_name_resolver.hpp>

namespace orcus { namespace spreadsheet {

namespace {

class table_auto_filter : public iface::import_auto_filter
{
    string_pool& m_pool;
    sheet_t m_sheet_pos;
    col_t m_cur_col;
    auto_filter_column_t m_cur_col_data;
    auto_filter_t m_filter_data;

    auto_filter_t* mp_data;

public:
    table_auto_filter(document& doc, sheet& sh) :
        m_pool(doc.get_string_pool()),
        m_sheet_pos(sh.get_index()),
        m_cur_col(-1),
        mp_data(nullptr) {}

    void reset(auto_filter_t* data)
    {
        mp_data = data;
        m_cur_col = -1;
        m_cur_col_data.reset();
        m_filter_data.reset();
    }

    virtual void set_range(const range_t& range)
    {
        m_filter_data.range = to_abs_range(range, m_sheet_pos);
    }

    virtual void set_column(orcus::spreadsheet::col_t col)
    {
        m_cur_col = col;
    }

    virtual void append_column_match_value(const char* p, size_t n)
    {
        // The string pool belongs to the document.
        pstring s = m_pool.intern(p, n).first;
        m_cur_col_data.match_values.insert(s);
    };

    virtual void commit_column()
    {
        m_filter_data.commit_column(m_cur_col, m_cur_col_data);
        m_cur_col_data.reset();
    }

    virtual void commit()
    {
        if (!mp_data)
            return;

        mp_data->swap(m_filter_data);
    }
};

}

struct import_table::impl
{
    document& m_doc;
    sheet& m_sheet;

    table_auto_filter m_auto_filter;

    std::unique_ptr<table_t> mp_data;
    table_column_t m_column;

    impl(const impl&) = delete;
    impl& operator=(const impl&) = delete;

    impl(document& doc, sheet& sh) :
        m_doc(doc), m_sheet(sh), m_auto_filter(doc, sh) {}
};

import_table::import_table(document& doc, sheet& sh) : mp_impl(orcus::make_unique<impl>(doc, sh)) {}

import_table::~import_table() {}

iface::import_auto_filter* import_table::get_auto_filter()
{
    mp_impl->m_auto_filter.reset(&mp_impl->mp_data->filter);
    return &mp_impl->m_auto_filter;
}

void import_table::set_range(const char* p_ref, size_t n_ref)
{
    const ixion::formula_name_resolver* resolver = mp_impl->m_doc.get_formula_name_resolver();
    if (!resolver)
        return;

    ixion::abs_range_t& range = mp_impl->mp_data->range;
    range = to_abs_range(*resolver, p_ref, n_ref);
    if (range.valid())
        range.first.sheet = range.last.sheet = mp_impl->m_sheet.get_index();
}

void import_table::set_identifier(size_t id)
{
    mp_impl->mp_data->identifier = id;
}

void import_table::set_name(const char* p, size_t n)
{
    string_pool& sp = mp_impl->m_doc.get_string_pool();
    mp_impl->mp_data->name = sp.intern(p, n).first;
}

void import_table::set_display_name(const char* p, size_t n)
{
    string_pool& sp = mp_impl->m_doc.get_string_pool();
    mp_impl->mp_data->display_name = sp.intern(p, n).first;
}

void import_table::set_totals_row_count(size_t row_count)
{
    mp_impl->mp_data->totals_row_count = row_count;
}

void import_table::set_column_count(size_t n)
{
    mp_impl->mp_data->columns.reserve(n);
}

void import_table::set_column_identifier(size_t id)
{
    mp_impl->m_column.identifier = id;
}

void import_table::set_column_name(const char* p, size_t n)
{
    string_pool& sp = mp_impl->m_doc.get_string_pool();
    mp_impl->m_column.name = sp.intern(p, n).first;
}

void import_table::set_column_totals_row_label(const char* p, size_t n)
{
    string_pool& sp = mp_impl->m_doc.get_string_pool();
    mp_impl->m_column.totals_row_label = sp.intern(p, n).first;
}

void import_table::set_column_totals_row_function(orcus::spreadsheet::totals_row_function_t func)
{
    mp_impl->m_column.totals_row_function = func;
}

void import_table::commit_column()
{
    mp_impl->mp_data->columns.push_back(mp_impl->m_column);
    mp_impl->m_column.reset();
}

void import_table::set_style_name(const char* p, size_t n)
{
    table_style_t& style = mp_impl->mp_data->style;
    string_pool& sp = mp_impl->m_doc.get_string_pool();
    style.name = sp.intern(p, n).first;
}

void import_table::set_style_show_first_column(bool b)
{
    table_style_t& style = mp_impl->mp_data->style;
    style.show_first_column = b;
}

void import_table::set_style_show_last_column(bool b)
{
    table_style_t& style = mp_impl->mp_data->style;
    style.show_last_column = b;
}

void import_table::set_style_show_row_stripes(bool b)
{
    table_style_t& style = mp_impl->mp_data->style;
    style.show_row_stripes = b;
}

void import_table::set_style_show_column_stripes(bool b)
{
    table_style_t& style = mp_impl->mp_data->style;
    style.show_column_stripes = b;
}

void import_table::commit()
{
    mp_impl->m_doc.insert_table(mp_impl->mp_data.release());
    mp_impl->mp_data.reset(new table_t);
}

void import_table::reset()
{
    mp_impl->mp_data.reset(new table_t);
    mp_impl->m_column.reset();
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
