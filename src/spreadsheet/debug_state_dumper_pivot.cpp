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

void debug_state_dumper_pivot_table::dump(const fs::path& outpath) const
{
    std::ofstream of{outpath.string()};

    if (!of)
        return;

    of << "name: " << m_store.name << "\n";
    of << "cache-id: " << m_store.cache_id << "\n";
    of << "range: " << m_cxt.print_range(m_store.range) << "\n";
    of << "fields:" << std::endl;

    for (const auto& field : m_store.fields)
    {
        of << "  - axis: " << field.axis << "\n";
        of << "    items: " << std::endl;

        for (const auto& item : field.items)
        {
            of << "      - ";
            std::ostringstream os;

            switch (item.type)
            {
                case pivot_item_t::item_type::index:
                {
                    os << "(" << std::get<std::size_t>(item.value);

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
            of << std::endl;
        }
    }

    of << "row-fields:" << std::endl;

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

    of << "column-fields:" << std::endl;

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

    of << "page-fields:" << std::endl;

    for (const auto& field : m_store.page_fields)
    {
        of << "  - field: (" << field.field << ")\n";
        of << "    item: ";

        if (field.item)
            of << "(" << *field.item << ")";
        else
            of << "(not set)";

        of << std::endl;
    }
}

}}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
