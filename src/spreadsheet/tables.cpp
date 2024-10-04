/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <orcus/spreadsheet/tables.hpp>
#include <orcus/spreadsheet/table.hpp>
#include <orcus/spreadsheet/document.hpp>
#include <orcus/string_pool.hpp>

#include <ixion/model_context.hpp>
#include <ixion/interface/table_handler.hpp>

#include <map>
#include <sstream>
#include <algorithm>

namespace orcus { namespace spreadsheet {

namespace {

using table_store_type = std::map<std::string_view, std::shared_ptr<table_t>>;

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
            range = ixion::abs_range_t{};
            return;
        }

        range.first.row = range.last.row - tab.totals_row_count - 1;
        return;
    }

    // Empty range.
    range = ixion::abs_range_t{};
}

class ixion_table_handler : public ixion::iface::table_handler
{
    const ixion::model_context& m_context;
    const table_store_type& m_tables;

    const table_t* find_table(const ixion::abs_address_t& pos) const
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

    std::string_view get_string(ixion::string_id_t sid) const
    {
        if (sid == ixion::empty_string_id)
            return std::string_view{};

        const std::string* p = m_context.get_string(sid);
        if (!p || p->empty())
            return std::string_view{};

        return std::string_view(p->data(), p->size());
    }

    col_t find_column(const table_t& tab, std::string_view name, size_t offset) const
    {
        if (offset >= tab.columns.size())
            return -1;

        auto it_beg = tab.columns.cbegin();
        auto it_end = tab.columns.cend();

        std::advance(it_beg, offset);
        auto it = std::find_if(it_beg, it_end, [name](const table_column_t& col)
        {
            return col.name == name;
        });

        if (it == it_end)
            // not found.
            return -1;

        size_t dist = std::distance(tab.columns.begin(), it);
        return tab.range.first.column + dist;
    }

    ixion::abs_range_t get_range_from_table(
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
                if (std::string_view col2_name = get_string(column_last); !col2_name.empty())
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

        return ixion::abs_range_t{};
    }

public:

    ixion_table_handler(const ixion::model_context& cxt, const table_store_type& tables) :
        m_context(cxt), m_tables(tables) {}

    ixion::abs_range_t get_range(
        const ixion::abs_address_t& pos,
        ixion::string_id_t column_first,
        ixion::string_id_t column_last,
        ixion::table_areas_t areas) const override
    {
        const table_t* tab = find_table(pos);
        if (!tab)
            return ixion::abs_range_t(ixion::abs_range_t::invalid);

        return get_range_from_table(*tab, column_first, column_last, areas);
    }

    ixion::abs_range_t get_range(
        ixion::string_id_t table,
        ixion::string_id_t column_first,
        ixion::string_id_t column_last,
        ixion::table_areas_t areas) const override
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
};

} // anonymous namespace

struct tables::impl
{
    string_pool& sp;
    table_store_type store;

    ixion_table_handler handler;

    impl(string_pool& _sp, ixion::model_context& context) :
        sp(_sp),
        handler(context, store)
    {
        context.set_table_handler(&handler);
    }
};

tables::tables(string_pool& sp, ixion::model_context& context) : mp_impl(std::make_unique<impl>(sp, context)) {}
tables::~tables() = default;

void tables::insert(std::unique_ptr<table_t> p)
{
    if (!p)
        throw std::invalid_argument("null table_t instance is not allowed");

    if (!p->range.valid())
    {
        std::ostringstream os;
        os << "table range is invalid: " << p->range;
        throw std::invalid_argument(os.str());
    }

    if (p->range.first.sheet != p->range.last.sheet)
        throw std::invalid_argument("one table can only belong to one sheet only");

    std::string_view name = p->name;
    mp_impl->store.emplace(name, std::move(p));
}

std::weak_ptr<const table_t> tables::get(std::string_view name) const
{
    auto it = mp_impl->store.find(name);
    if (it == mp_impl->store.end())
        return {};

    return it->second;
}

std::map<std::string_view, std::weak_ptr<const table_t>> tables::get_by_sheet(sheet_t pos) const
{
    std::map<std::string_view, std::weak_ptr<const table_t>> ret;

    for (const auto& [name, tab] : mp_impl->store)
    {
        if (tab->range.first.sheet != pos)
            continue;

        std::weak_ptr<const table_t> p(tab);
        ret.insert_or_assign(name, std::move(p));
    }

    return ret;
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
