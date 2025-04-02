/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "factory_pivot_table_def.hpp"
#include "formula_global.hpp"

#include <orcus/spreadsheet/document.hpp>
#include <ixion/address.hpp>
#include <iostream>

namespace orcus { namespace spreadsheet { namespace detail {

void import_pivot_field::set_item_count(std::size_t count)
{
    m_current_field.items.reserve(count);
}

void import_pivot_field::set_axis(pivot_axis_t axis)
{
    m_current_field.axis = axis;
}

void import_pivot_field::append_item(std::size_t index)
{
    m_current_field.items.emplace_back(index);
}

void import_pivot_field::append_item(pivot_field_item_t type)
{
    m_current_field.items.emplace_back(type);
}

void import_pivot_field::commit()
{
    m_func(std::move(m_current_field));
}

void import_pivot_field::reset(commit_func_type func)
{
    m_func = std::move(func);
    m_current_field = pivot_field_t{};
}

void import_pivot_fields::set_count(std::size_t count)
{
    m_current_fields.reserve(count);
}

iface::import_pivot_field* import_pivot_fields::start_pivot_field()
{
    m_xfield.reset([this](pivot_field_t&& field) { m_current_fields.push_back(std::move(field)); });
    return &m_xfield;
}

void import_pivot_fields::commit()
{
    m_func(std::move(m_current_fields));
}

void import_pivot_fields::reset(commit_func_type func)
{
    m_func = std::move(func);
    m_current_fields.clear();
}

void import_pivot_rc_fields::set_count(std::size_t count) {}

void import_pivot_rc_fields::append_field(std::size_t index) {}

void import_pivot_rc_fields::commit() {}

void import_pivot_page_field::set_field(std::size_t index) {}

void import_pivot_page_field::commit() {}

iface::import_pivot_page_field* import_pivot_page_fields::start_page_field()
{
    return &m_field;
}

void import_pivot_page_fields::commit() {}

void import_pivot_data_field::set_field(std::size_t index) {}

void import_pivot_data_field::commit() {}

iface::import_pivot_data_field* import_pivot_data_fields::start_data_field()
{
    return &m_field;
}

void import_pivot_data_fields::commit() {}

void import_pivot_rc_item::append_index(std::size_t index) {}

void import_pivot_rc_item::commit() {}

iface::import_pivot_rc_item* import_pivot_rc_items::start_item()
{
    return &m_item;
}

void import_pivot_rc_items::commit() {}

import_pivot_table_def::import_pivot_table_def(document& doc) :
    m_doc(doc),
    m_current_pt(doc.get_string_pool())
{}

void import_pivot_table_def::set_name(std::string_view name)
{
    m_current_pt.set_name(name);
}

void import_pivot_table_def::set_cache_id(pivot_cache_id_t cache_id)
{
    m_current_pt.set_cache_id(cache_id);
}

void import_pivot_table_def::set_range(const range_t& ref)
{
    m_current_pt.set_range(to_abs_rc_range(ref));
}

iface::import_pivot_fields* import_pivot_table_def::start_pivot_fields()
{
    m_pivot_fields.reset([this](pivot_fields_t&& fields) { m_current_pt.set_pivot_fields(std::move(fields)); });
    return &m_pivot_fields;
}

iface::import_pivot_rc_fields* import_pivot_table_def::start_row_fields()
{
    return &m_rc_fields;
}

iface::import_pivot_rc_fields* import_pivot_table_def::start_col_fields()
{
    return &m_rc_fields;
}

iface::import_pivot_page_fields* import_pivot_table_def::start_page_fields()
{
    return &m_page_fields;
}

iface::import_pivot_data_fields* import_pivot_table_def::start_data_fields()
{
    return &m_data_fields;
}

iface::import_pivot_rc_items* import_pivot_table_def::start_row_items()
{
    return &m_rc_items;
}

iface::import_pivot_rc_items* import_pivot_table_def::start_col_items()
{
    return &m_rc_items;
}

void import_pivot_table_def::commit()
{
    m_doc.get_pivot_collection().insert_pivot_table(std::move(m_current_pt));
}

void import_pivot_table_def::reset()
{
    m_current_pt = pivot_table(m_doc.get_string_pool());
}

}}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
