/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "document_impl.hpp"

namespace orcus { namespace spreadsheet { namespace detail {

namespace {

class find_column_by_name
{
    std::string_view m_name;
public:
    find_column_by_name(std::string_view name) : m_name(name) {}

    bool operator() (const table_column_t& col) const
    {
        return col.name == m_name;
    }
};

void adjust_row_range(ixion::abs_range_t& range, const table_t& tab, ixion::table_areas_t areas)
{
    bool headers = (areas & ixion::table_area_headers);
    bool data    = (areas & ixion::table_area_data);
    bool totals  = (areas & ixion::table_area_totals);

    if (headers)
    {
        if (data)
        {
            if (totals)
            {
                // All areas.
                return;
            }

            // Headers + data
            range.last.row -= tab.totals_row_count;
            return;
        }

        if (totals)
        {
            // Header + total is invalid.
            range = ixion::abs_range_t(ixion::abs_range_t::invalid);
            return;
        }

        // Headers only.
        range.last.row = range.first.row;
        return;
    }

    if (data)
    {
        ++range.first.row;

        if (totals)
        {
            // Data + total
            return;
        }

        // Data only
        range.last.row -= tab.totals_row_count;
        return;
    }

    if (totals)
    {
        // Total only
        if (!tab.totals_row_count)
        {
            // This table has not total rows.  Return empty range.
            range = ixion::abs_range_t();
            return;
        }

        range.first.row = range.last.row - tab.totals_row_count - 1;
        return;
    }

    // Empty range.
    range = ixion::abs_range_t();
}

}

sheet_item::sheet_item(document& doc, std::string_view _name, sheet_t sheet_index) :
    name(_name), data(doc, sheet_index) {}


const table_t* ixion_table_handler::find_table(const ixion::abs_address_t& pos) const
{
    auto it = m_tables.begin(), it_end = m_tables.end();
    for (; it != it_end; ++it)
    {
        const table_t* p = it->second.get();
        if (p->range.contains(pos))
            return p;
    }

    return nullptr;
}

std::string_view ixion_table_handler::get_string(ixion::string_id_t sid) const
{
    if (sid == ixion::empty_string_id)
        return std::string_view{};

    const std::string* p = m_context.get_string(sid);
    if (!p || p->empty())
        return std::string_view{};

    return std::string_view(p->data(), p->size());
}

col_t ixion_table_handler::find_column(const table_t& tab, std::string_view name, size_t offset) const
{
    if (offset >= tab.columns.size())
        return -1;

    table_t::columns_type::const_iterator it_beg = tab.columns.begin();
    table_t::columns_type::const_iterator it_end = tab.columns.end();

    std::advance(it_beg, offset);
    table_t::columns_type::const_iterator it =
        std::find_if(it_beg, it_end, find_column_by_name(name));

    if (it == it_end)
        // not found.
        return -1;

    size_t dist = std::distance(tab.columns.begin(), it);
    return tab.range.first.column + dist;
}

ixion::abs_range_t ixion_table_handler::get_range_from_table(
    const table_t& tab, ixion::string_id_t column_first, ixion::string_id_t column_last,
    ixion::table_areas_t areas) const
{
    if (column_first != ixion::empty_string_id)
    {
        std::string_view col1_name = get_string(column_first);
        if (col1_name.empty())
            return ixion::abs_range_t(ixion::abs_range_t::invalid);

        col_t col1_index = find_column(tab, col1_name, 0);
        if (col1_index < 0)
            return ixion::abs_range_t(ixion::abs_range_t::invalid);

        if (column_last != ixion::empty_string_id)
        {
            std::string_view col2_name = get_string(column_last);
            if (!col2_name.empty())
            {
                // column range table reference.
                col_t col2_index = find_column(tab, col2_name, col1_index);
                ixion::abs_range_t range = tab.range;
                range.first.column = col1_index;
                range.last.column = col2_index;
                adjust_row_range(range, tab, areas);
                return range;
            }
        }

        // single column table reference.
        ixion::abs_range_t range = tab.range;
        range.first.column = range.last.column = col1_index;
        adjust_row_range(range, tab, areas);
        return range;
    }

    return ixion::abs_range_t();
}

ixion_table_handler::ixion_table_handler(const ixion::model_context& cxt, const table_store_type& tables) :
    m_context(cxt), m_tables(tables) {}

ixion::abs_range_t ixion_table_handler::get_range(
    const ixion::abs_address_t& pos, ixion::string_id_t column_first, ixion::string_id_t column_last,
    ixion::table_areas_t areas) const
{
    const table_t* tab = find_table(pos);
    if (!tab)
        return ixion::abs_range_t(ixion::abs_range_t::invalid);

    return get_range_from_table(*tab, column_first, column_last, areas);

}

ixion::abs_range_t ixion_table_handler::get_range(
    ixion::string_id_t table, ixion::string_id_t column_first, ixion::string_id_t column_last,
    ixion::table_areas_t areas) const
{
    std::string_view tab_name = get_string(table);
    if (tab_name.empty())
        // no table name given.
        return ixion::abs_range_t(ixion::abs_range_t::invalid);

    auto it = m_tables.find(tab_name);
    if (it == m_tables.end())
        // no table by this name found.
        return ixion::abs_range_t(ixion::abs_range_t::invalid);

    const table_t* tab = it->second.get();
    return get_range_from_table(*tab, column_first, column_last, areas);
}

document_impl::document_impl(document& _doc, const range_size_t& sheet_size) :
    doc(_doc),
    context({sheet_size.rows, sheet_size.columns}),
    styles_store(),
    ss_store(string_pool_store, context, styles_store),
    pivots(doc),
    name_resolver_global(ixion::formula_name_resolver::get(ixion::formula_name_resolver_t::excel_a1, &context)),
    grammar(formula_grammar_t::xlsx),
    table_handler(context, tables)
{
    context.set_table_handler(&table_handler);
}

}}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
