/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "factory_pivot.hpp"

#include "orcus/global.hpp"
#include "orcus/string_pool.hpp"
#include "orcus/exception.hpp"

#include <sstream>
#include <iostream>

namespace orcus { namespace spreadsheet {

class import_pc_field_group : public iface::import_pivot_cache_field_group
{
    using range_grouping_type = pivot_cache_group_data_t::range_grouping_type;

    document& m_doc;
    pivot_cache_field_t& m_parent_field;
    std::unique_ptr<pivot_cache_group_data_t> m_data;
    pivot_cache_item_t m_current_field_item;

private:
    pstring intern(const char* p, size_t n)
    {
        return m_doc.get_string_pool().intern(p, n).first;
    }

    range_grouping_type& get_range_grouping()
    {
        if (!m_data->range_grouping)
            m_data->range_grouping = range_grouping_type();

        return *m_data->range_grouping;
    }

public:
    import_pc_field_group(document& doc, pivot_cache_field_t& parent, size_t base_index) :
        m_doc(doc),
        m_parent_field(parent),
        m_data(std::make_unique<pivot_cache_group_data_t>(base_index)) {}

    ~import_pc_field_group() override {}

    void link_base_to_group_items(size_t group_item_index) override
    {
        pivot_cache_indices_t& b2g = m_data->base_to_group_indices;
        b2g.push_back(group_item_index);
    }

    void set_field_item_string(const char* p, size_t n) override
    {
        m_current_field_item.type = pivot_cache_item_t::item_type::character;
        pstring s = intern(p, n);
        m_current_field_item.value.character.p = s.get();
        m_current_field_item.value.character.n = s.size();
    }

    void set_field_item_numeric(double v) override
    {
        m_current_field_item.type = pivot_cache_item_t::item_type::numeric;
        m_current_field_item.value.numeric = v;
    }

    void commit_field_item() override
    {
        m_data->items.push_back(std::move(m_current_field_item));
    }

    void set_range_grouping_type(pivot_cache_group_by_t group_by) override
    {
        get_range_grouping().group_by = group_by;
    }

    void set_range_auto_start(bool b) override
    {
        get_range_grouping().auto_start = b;
    }

    void set_range_auto_end(bool b) override
    {
        get_range_grouping().auto_end = b;
    }

    void set_range_start_number(double v) override
    {
        get_range_grouping().start = v;
    }

    void set_range_end_number(double v) override
    {
        get_range_grouping().end = v;
    }

    void set_range_start_date(const date_time_t& dt) override
    {
        get_range_grouping().start_date = dt;
    }

    void set_range_end_date(const date_time_t& dt) override
    {
        get_range_grouping().end_date = dt;
    }

    void set_range_interval(double v) override
    {
        get_range_grouping().interval = v;
    }

    void commit() override
    {
        m_parent_field.group_data = std::move(m_data);
    }
};

pstring import_pivot_cache_def::intern(const char* p, size_t n)
{
    return m_doc.get_string_pool().intern(p, n).first;
}

import_pivot_cache_def::import_pivot_cache_def(document& doc) : m_doc(doc) {}

import_pivot_cache_def::~import_pivot_cache_def() {}

void import_pivot_cache_def::create_cache(pivot_cache_id_t cache_id)
{
    m_src_type = unknown;
    m_cache = std::make_unique<pivot_cache>(cache_id, m_doc.get_string_pool());
}

void import_pivot_cache_def::set_worksheet_source(
    const char* ref, size_t n_ref, const char* sheet_name, size_t n_sheet_name)
{
    assert(m_cache);

    const ixion::formula_name_resolver* resolver =
        m_doc.get_formula_name_resolver(spreadsheet::formula_ref_context_t::global);
    assert(resolver);

    m_src_type = worksheet;
    m_src_sheet_name = intern(sheet_name, n_sheet_name);

    ixion::formula_name_t fn = resolver->resolve(ref, n_ref, ixion::abs_address_t(0,0,0));

    if (fn.type != ixion::formula_name_t::range_reference)
    {
        std::ostringstream os;
        os << pstring(ref, n_ref) << " is not a valid range.";
        throw xml_structure_error(os.str());
    }

    m_src_range = ixion::to_range(fn.range).to_abs(ixion::abs_address_t(0,0,0));
}

void import_pivot_cache_def::set_worksheet_source(const char* table_name, size_t n_table_name)
{
    assert(m_cache);

    m_src_table_name = intern(table_name, n_table_name);
}

void import_pivot_cache_def::set_field_count(size_t n)
{
    m_current_fields.reserve(n);
}

void import_pivot_cache_def::set_field_name(const char* p, size_t n)
{
    m_current_field.name = intern(p, n);
}

iface::import_pivot_cache_field_group* import_pivot_cache_def::create_field_group(size_t base_index)
{
    m_current_field_group =
        std::make_unique<import_pc_field_group>(m_doc, m_current_field, base_index);

    return m_current_field_group.get();
}

void import_pivot_cache_def::set_field_min_value(double v)
{
    m_current_field.min_value = v;
}

void import_pivot_cache_def::set_field_max_value(double v)
{
    m_current_field.max_value = v;
}

void import_pivot_cache_def::set_field_min_date(const date_time_t& dt)
{
    m_current_field.min_date = dt;
}

void import_pivot_cache_def::set_field_max_date(const date_time_t& dt)
{
    m_current_field.max_date = dt;
}

void import_pivot_cache_def::commit_field()
{
    m_current_fields.push_back(std::move(m_current_field));
}

void import_pivot_cache_def::set_field_item_string(const char* p, size_t n)
{
    m_current_field_item.type = pivot_cache_item_t::item_type::character;
    pstring s = intern(p, n);
    m_current_field_item.value.character.p = s.get();
    m_current_field_item.value.character.n = s.size();
}

void import_pivot_cache_def::set_field_item_numeric(double v)
{
    m_current_field_item.type = pivot_cache_item_t::item_type::numeric;
    m_current_field_item.value.numeric = v;
}

void import_pivot_cache_def::set_field_item_date_time(const date_time_t& dt)
{
    m_current_field_item.type = pivot_cache_item_t::item_type::date_time;
    m_current_field_item.value.date_time.year = dt.year;
    m_current_field_item.value.date_time.month = dt.month;
    m_current_field_item.value.date_time.day = dt.day;
    m_current_field_item.value.date_time.hour = dt.hour;
    m_current_field_item.value.date_time.minute = dt.minute;
    m_current_field_item.value.date_time.second = dt.second;
}

void import_pivot_cache_def::set_field_item_error(error_value_t ev)
{
    m_current_field_item.type = pivot_cache_item_t::item_type::error;
    m_current_field_item.value.error = ev;
}

void import_pivot_cache_def::commit_field_item()
{
    m_current_field.items.push_back(std::move(m_current_field_item));
}

void import_pivot_cache_def::commit()
{
    m_cache->insert_fields(std::move(m_current_fields));
    assert(m_current_fields.empty());

    if (!m_src_table_name.empty())
    {
        m_doc.get_pivot_collection().insert_worksheet_cache(
            m_src_table_name, std::move(m_cache));
        return;
    }

    m_doc.get_pivot_collection().insert_worksheet_cache(
        m_src_sheet_name, m_src_range, std::move(m_cache));
}

import_pivot_cache_records::import_pivot_cache_records(document& doc) :
    m_doc(doc), m_cache(nullptr) {}

import_pivot_cache_records::~import_pivot_cache_records() {}

void import_pivot_cache_records::set_cache(pivot_cache* p)
{
    m_cache = p;
}

void import_pivot_cache_records::set_record_count(size_t n)
{
    m_records.reserve(n);
}

void import_pivot_cache_records::append_record_value_numeric(double v)
{
    m_current_record.emplace_back(v);
}

void import_pivot_cache_records::append_record_value_character(const char* p, size_t n)
{
    m_current_record.emplace_back(p, n);
}

void import_pivot_cache_records::append_record_value_shared_item(size_t index)
{
    m_current_record.emplace_back(index);
}

void import_pivot_cache_records::commit_record()
{
    if (!m_cache)
    {
        m_current_record.clear();
        return;
    }

    m_records.push_back(std::move(m_current_record));
}

void import_pivot_cache_records::commit()
{
    if (!m_cache)
        return;

    m_cache->insert_records(std::move(m_records));
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
