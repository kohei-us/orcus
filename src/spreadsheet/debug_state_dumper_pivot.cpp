/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "debug_state_dumper_pivot.hpp"
#include "debug_state_context.hpp"
#include "pivot_impl.hpp"

#include <sstream>
#include <fstream>

namespace ss = orcus::spreadsheet;

namespace orcus { namespace spreadsheet { namespace detail {

debug_state_dumper_pivot_cache::debug_state_dumper_pivot_cache(const pivot_cache::impl& store) :
    m_store(store)
{
}

void debug_state_dumper_pivot_cache::dump(const fs::path& outdir) const
{
    fs::create_directories(outdir);

    std::ostringstream os;
    os << "cache-" << m_store.cache_id << ".yaml";
    fs::path outpath = outdir / os.str();
    std::ofstream of{outpath.native()};

    if (!of)
        return;

    of << "id: " << m_store.cache_id << std::endl;
    of << "fields:" << std::endl;

    for (const auto& field : m_store.fields)
    {
        of << "  - name: " << field.name << std::endl;

        if (!field.items.empty())
        {
            of << "    items:" << std::endl;
            for (const auto& item : field.items)
                of << "      - " << item << std::endl;
        }

        if (field.min_value)
            of << "    min value: " << *field.min_value << std::endl;

        if (field.max_value)
            of << "    max value: " << *field.max_value << std::endl;

        if (field.min_date)
            of << "    min date: " << field.min_date->to_string() << std::endl;

        if (field.max_date)
            of << "    max date: " << field.max_date->to_string() << std::endl;

        if (field.group_data)
        {
            const pivot_cache_group_data_t& data = *field.group_data;
            of << "    group:\n";
            of << "      base field:\n";
            of << "        index: " << data.base_field << std::endl;

            const pivot_cache_field_t* base_field = nullptr;

            if (data.base_field < m_store.fields.size())
            {
                base_field = &m_store.fields[data.base_field];
                of << "        name: " << base_field->name << std::endl;
            }

            if (!data.base_to_group_indices.empty())
            {
                of << "      base item references:" << std::endl;
                for (auto v : data.base_to_group_indices)
                {
                    of << "        - (" << v << ")";

                    if (base_field && v < base_field->items.size())
                        of << " -> '" << base_field->items[v] << "'";

                    of << std::endl;
                }
            }

            if (!data.items.empty())
            {
                of << "      items:" << std::endl;
                for (const auto& item : data.items)
                    of << "        - " << item << std::endl;
            }

            if (data.range_grouping)
            {
                const auto& rg = *data.range_grouping;
                of << "      range grouping:" << "\n";
                of << "        group by: " << rg.group_by << "\n";
                of << "        auto start: " << std::boolalpha << rg.auto_start << "\n";
                of << "        auto end: " << std::boolalpha << rg.auto_end << "\n";
                of << "        start: " << rg.start << "\n";
                of << "        end: " << rg.end << "\n";
                of << "        interval: " << rg.interval << "\n";
                of << "        start date: " << rg.start_date.to_string() << "\n";
                of << "        end date: " << rg.end_date.to_string() << std::endl;
            }
        }
    }

    of << "records:" << std::endl;

    for (const auto& record : m_store.records)
    {
        for (std::size_t i = 0; i < record.size(); ++i)
        {
            if (i == 0)
                of << "  - ";
            else
                of << "    ";

            const auto& v = record[i];
            of << "- " << v;

            if (v.type == pivot_cache_record_value_t::record_type::shared_item_index && i < m_store.fields.size())
            {
                auto sii = std::get<std::size_t>(v.value);
                if (sii < m_store.fields[i].items.size())
                    of << " -> '" << m_store.fields[i].items[sii] << "'";
            }

            of << std::endl;
        }
    }
}

debug_state_dumper_pivot_table::debug_state_dumper_pivot_table(
    const debug_state_context& cxt, const pivot_table::impl& store) :
    m_cxt(cxt), m_store(store) {}

void debug_state_dumper_pivot_table::dump(
    const fs::path& outpath, const detail::caches_type& caches) const
{
    std::ofstream of{outpath.string()};

    if (!of)
        return;

    const auto* cache_store = get_cache_store(caches, m_store.cache_id);
    if (!cache_store)
        return;

    of << "name: " << m_store.name << "\n";
    of << "cache-id: " << m_store.cache_id << "\n";
    of << "range: " << m_cxt.print_range(m_store.range) << "\n";
    of << "fields:\n";

    for (const auto& field : m_store.fields)
    {
        of << "  - axis: " << field.axis << "\n";
        of << "    items:\n";

        for (const auto& item : field.items)
        {
            of << "      - ";
            std::ostringstream os;

            switch (item.type)
            {
                case pivot_item_t::item_type::index:
                {
                    auto v = std::get<std::size_t>(item.value);
                    os << "(" << v;

                    if (item.hidden)
                        os << "; hidden)";
                    else
                        os << ")";

                    break;
                }
                case pivot_item_t::item_type::type:
                {
                    os << std::get<pivot_field_item_t>(item.value);
                    break;
                }
                case pivot_item_t::item_type::unknown:
                {
                    os << "???";
                    break;
                }
            }

            m_cxt.ensure_yaml_string(of, os.str());
            of << "\n";
        }
    }

    of << "row-fields:\n";

    for (const auto& field : m_store.row_fields)
    {
        switch (field.type)
        {
            case pivot_ref_rc_field_t::value_type::index:
                of << "  - (" << field.index << ")\n";
                break;
            case pivot_ref_rc_field_t::value_type::data:
                of << "  - (data)\n";
                break;
            case pivot_ref_rc_field_t::value_type::unknown:
                of << "  - (unknown)\n";
                break;
        }
    }

    of << "column-fields:\n";

    for (const auto& field : m_store.column_fields)
    {
        switch (field.type)
        {
            case pivot_ref_rc_field_t::value_type::index:
                of << "  - (" << field.index << ")\n";
                break;
            case pivot_ref_rc_field_t::value_type::data:
                of << "  - (data)\n";
                break;
            case pivot_ref_rc_field_t::value_type::unknown:
                of << "  - (unknown)\n";
                break;
        }
    }

    of << "page-fields:\n";

    for (const auto& field : m_store.page_fields)
    {
        of << "  - field: (" << field.field << ")\n";
        of << "    item: ";

        if (field.item)
            of << "(" << *field.item << ")";
        else
            of << "null";

        of << "\n";
    }

    of << "data-fields:\n";
    for (const auto& field : m_store.data_fields)
    {
        of << "  - field: (" << field.field << ")\n";
        of << "    name: " << field.name << "\n";
        of << "    subtotal: ";

        if (field.subtotal == ss::pivot_data_subtotal_t::unknown)
            of << "null";
        else
            of << field.subtotal;
        of << "\n";

        if (field.show_data_as == ss::pivot_data_show_data_as_t::unknown)
            of << "    show data as: null" << std::endl;
        else
        {
            // TODO: Show base field and base item only when the type uses them
            of << "    show data as:\n";
            of << "      type: " << field.show_data_as << std::endl;

            switch (field.show_data_as)
            {
                case pivot_data_show_data_as_t::difference:
                case pivot_data_show_data_as_t::percent:
                case pivot_data_show_data_as_t::percent_diff:
                    // these types use both base field and item
                    of << "      base field: (" << field.base_field << ")\n";
                    of << "      base item: (" << field.base_item << ")" << std::endl;
                    break;
                case pivot_data_show_data_as_t::percent_of_col:
                case pivot_data_show_data_as_t::percent_of_row:
                case pivot_data_show_data_as_t::percent_of_total:
                    // these types only use base field
                    of << "      base field: (" << field.base_field << ")" << std::endl;
                    break;
                case pivot_data_show_data_as_t::normal:
                case pivot_data_show_data_as_t::index:
                case pivot_data_show_data_as_t::run_total:
                case pivot_data_show_data_as_t::unknown:
                    // these types don't use base field nor item
                    break;
            }
        }
    }

    of << "row-items:\n";
    dump_rc_items(of, m_store.row_items);
    of << "column-items:\n";
    dump_rc_items(of, m_store.column_items);
    of << std::endl;
}

void debug_state_dumper_pivot_table::dump_rc_items(
    std::ofstream& of, const pivot_ref_rc_items_t& rc_items) const
{
    for (const auto& item : rc_items)
    {
        of << "  - type: " << item.type << "\n";
        of << "    data-item: ";

        if (item.data_item)
            of << '(' << *item.data_item << ')';
        else
            of << "null";
        of << "\n";

        of << "    item:\n";

        for (std::size_t i = 0; i < item.repeat; ++i)
            of << "      - null\n";

        for (auto v : item.items)
            of << "      - (" << v << ")\n";
    }
}

const pivot_cache::impl* debug_state_dumper_pivot_table::get_cache_store(
    const detail::caches_type& caches, pivot_cache_id_t cache_id) const
{
    auto it = caches.find(cache_id);
    if (it == caches.end())
        return nullptr;

    return it->second->mp_impl.get();
}

}}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
