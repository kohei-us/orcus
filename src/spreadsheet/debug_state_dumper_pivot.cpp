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

    of << "name: " << m_store.name << "\n";
    of << "cache-id: " << m_store.cache_id << "\n";
    of << "range: " << m_cxt.print_range(m_store.range) << "\n";

    const auto* cache_store = get_cache_store(caches, m_store.cache_id);
    if (!cache_store)
    {
        of << "# ERROR: cache store not found for cache ID of " << m_store.cache_id << std::endl;
        return;
    }

    if (cache_store->fields.size() != m_store.fields.size())
    {
        of << "# ERROR: field counts differ between the pivot table and the referenced pivot cache" << std::endl;
        return;
    }

    of << "fields:\n";

    for (std::size_t i = 0; i < m_store.fields.size(); ++i)
    {
        const auto& field = m_store.fields[i];
        const auto& pc_field = cache_store->fields[i];

        of << "  - name: ";
        m_cxt.ensure_yaml_string(of, pc_field.name);
        of << "\n";

        of << "    axis: " << field.axis << "\n";
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

                    os << " -> '";
                    os << pc_field.items[v];
                    os << "'";
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
    dump_axis_rc_fields(of, m_store.row_fields, *cache_store);

    of << "column-fields:\n";
    dump_axis_rc_fields(of, m_store.column_fields, *cache_store);

    of << "page-fields:\n";

    for (const auto& field : m_store.page_fields)
    {
        of << "  - field: ";
        m_cxt.ensure_yaml_string(of, create_ref_field_value(field.field, *cache_store));
        of << "\n";

        of << "    item: ";

        if (field.item)
            m_cxt.ensure_yaml_string(of, create_ref_item_value(field.field, *field.item, *cache_store));
        else
            of << "null";

        of << "\n";
    }

    of << "data-fields:\n";
    for (const auto& field : m_store.data_fields)
    {
        of << "  - field: ";
        m_cxt.ensure_yaml_string(of, create_ref_field_value(field.field, *cache_store));
        of << "\n";

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
    dump_rc_items(of, m_store.row_items, m_store.row_fields, *cache_store);
    of << "column-items:\n";
    dump_rc_items(of, m_store.column_items, m_store.column_fields, *cache_store);
    of << std::endl;
}

std::string debug_state_dumper_pivot_table::create_ref_field_value(
    std::size_t index, const pivot_cache::impl& cache_store) const
{
    std::ostringstream os;
    os << "(" << index << ") -> ";

    if (index < m_store.fields.size())
    {
        assert(index < cache_store.fields.size());
        os << "'" << cache_store.fields[index].name << "'";
    }
    else
        os << "(out-of-bound)";

    return os.str();
}

std::string debug_state_dumper_pivot_table::create_ref_item_value(
    std::size_t field, std::size_t item, const pivot_cache::impl& cache_store) const
{
    std::ostringstream os;
    os << "(" << item << ") -> ";

    if (field < m_store.fields.size())
    {
        if (const auto& pt_items = m_store.fields[field].items; item < pt_items.size())
        {
            if (const auto& item_value = pt_items[item]; item_value.type == pivot_item_t::item_type::index)
            {
                // overwrite the item index
                item = std::get<std::size_t>(item_value.value);

                assert(field < cache_store.fields.size());
                const auto& pc_items = cache_store.fields[field].items;
                if (item < pc_items.size())
                {
                    os << "'" << pc_items[item] << "'";
                }
                else
                    os << "(out-of-bound item)";
            }
            else
                os << "(non-index item)";
        }
        else
            os << "(out-of-bound item)";
    }
    else
        os << "(out-of-bound field)";

    return os.str();
}

void debug_state_dumper_pivot_table::dump_rc_items(
    std::ofstream& of, const pivot_ref_rc_items_t& rc_items,
    const pivot_ref_rc_fields_t& rc_fields, const pivot_cache::impl& cache_store) const
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

        bool follow_ref = item.type == pivot_field_item_t::data &&
            (item.repeat + item.items.size()) <= rc_fields.size();

        for (std::size_t i = 0; i < item.repeat; ++i)
            of << "      - null\n";

        std::size_t field_pos = item.repeat;

        for (auto v : item.items)
        {
            of << "      - ";

            std::ostringstream os;
            os << "(" << v << ")";

            if (follow_ref)
            {
                os << " -> ";
                const auto& fid = rc_fields[field_pos++];

                switch (fid.type)
                {
                    case pivot_ref_rc_field_t::value_type::index:
                    {
                        if (fid.index < m_store.fields.size())
                        {
                            const auto& pt_fld = m_store.fields[fid.index];
                            assert(fid.index < cache_store.fields.size());
                            const auto& pc_fld = cache_store.fields[fid.index];

                            if (v < pt_fld.items.size())
                            {
                                const auto& pt_item = pt_fld.items[v];
                                if (pt_item.type == pivot_item_t::item_type::index)
                                {
                                    v = std::get<std::size_t>(pt_item.value);
                                    if (v < pc_fld.items.size())
                                        os << "'" << pc_fld.items[v] << "'";
                                    else
                                        os << "(out-of-bound item in cache)";
                                }
                                else
                                    os << "(non-index item)";

                            }
                            else
                                os << "(out-of-bound item)";
                        }
                        else
                            os << "(out-of-bound field)";

                        break;
                    }
                    case pivot_ref_rc_field_t::value_type::data:
                    {
                        os << "(data)";
                        break;
                    }
                    case pivot_ref_rc_field_t::value_type::unknown:
                    {
                        os << "(unknown)";
                        break;
                    }
                }
            }

            m_cxt.ensure_yaml_string(of, os.str());

            of << "\n";
        }
    }
}

void debug_state_dumper_pivot_table::dump_axis_rc_fields(
    std::ofstream& of, const pivot_ref_rc_fields_t& fields, const pivot_cache::impl& cache_store) const
{
    for (const auto& field : fields)
    {
        switch (field.type)
        {
            case pivot_ref_rc_field_t::value_type::index:
            {
                of << "  - ";
                m_cxt.ensure_yaml_string(of, create_ref_field_value(field.index, cache_store));
                of << "\n";
                break;
            }
            case pivot_ref_rc_field_t::value_type::data:
                of << "  - (data)\n";
                break;
            case pivot_ref_rc_field_t::value_type::unknown:
                of << "  - (unknown)\n";
                break;
        }
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
