/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "debug_state_dumper_pivot.hpp"
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
            of << "      base field index: " << data.base_field << std::endl;

            if (!data.base_to_group_indices.empty())
            {
                of << "      base item indices:" << std::endl;
                for (auto v : data.base_to_group_indices)
                    of << "        - " << v << std::endl;
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
}

}}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
