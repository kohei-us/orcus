/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "factory_pivot_table_def.hpp"
#include "formula_global.hpp"

#include <orcus/string_pool.hpp>
#include <orcus/spreadsheet/document.hpp>
#include <ixion/address.hpp>
#include <cassert>

namespace orcus { namespace spreadsheet { namespace detail {

void import_pivot_field::set_item_count(std::size_t count)
{
    m_current_field.items.reserve(count);
}

void import_pivot_field::set_axis(pivot_axis_t axis)
{
    m_current_field.axis = axis;
}

void import_pivot_field::append_item(std::size_t index, bool hidden)
{
    m_current_field.items.emplace_back(index, hidden);
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

void import_pivot_rc_fields::set_count(std::size_t count)
{
    m_fields.reserve(count);
}

void import_pivot_rc_fields::append_field(std::size_t index)
{
    m_fields.push_back(index);
}

void import_pivot_rc_fields::append_data_field()
{
    m_fields.push_back(pivot_ref_rc_field_t::value_type::data);
}

void import_pivot_rc_fields::commit()
{
    m_axis = pivot_axis_t::unknown;
    m_func(std::move(m_fields));
}

void import_pivot_rc_fields::reset(pivot_axis_t axis, commit_func_type func)
{
    assert(axis == pivot_axis_t::row || axis == pivot_axis_t::column);
    m_func = std::move(func);
    m_axis = axis;
    m_fields.clear();
}

void import_pivot_page_field::set_field(std::size_t index)
{
    m_current_field.field = index;
}

void import_pivot_page_field::set_item(std::size_t index)
{
    m_current_field.item = index;
}

void import_pivot_page_field::commit()
{
    m_func(std::move(m_current_field));
}

void import_pivot_page_field::reset(commit_func_type func)
{
    m_func = std::move(func);
    m_current_field = pivot_ref_page_field_t{};
}

void import_pivot_page_fields::set_count(std::size_t count)
{
    m_current_fields.reserve(count);
}

iface::import_pivot_page_field* import_pivot_page_fields::start_page_field()
{
    m_xfield.reset([this](pivot_ref_page_field_t&& field) {
       m_current_fields.push_back(std::move(field));
    });
    return &m_xfield;
}

void import_pivot_page_fields::commit()
{
    m_func(std::move(m_current_fields));
}

void import_pivot_page_fields::reset(commit_func_type func)
{
    m_func = std::move(func);
    m_current_fields.clear();
}

import_pivot_data_field::import_pivot_data_field(string_pool& pool) : m_pool(pool) {}

void import_pivot_data_field::set_field(std::size_t index)
{
    m_current_field.field = index;
}

void import_pivot_data_field::set_name(std::string_view name)
{
    m_current_field.name = m_pool.intern(name).first;
}

void import_pivot_data_field::set_subtotal_function(pivot_data_subtotal_t func)
{
    m_current_field.subtotal = func;
}

void import_pivot_data_field::set_show_data_as(
    pivot_data_show_data_as_t type, std::size_t base_field, std::size_t base_item)
{
    m_current_field.show_data_as = type;
    m_current_field.base_field = base_field;
    m_current_field.base_item = base_item;
}

void import_pivot_data_field::commit()
{
    m_func(std::move(m_current_field));
}

void import_pivot_data_field::reset(commit_func_type func)
{
    m_func = std::move(func);
    m_current_field = pivot_ref_data_field_t{};
}

import_pivot_data_fields::import_pivot_data_fields(string_pool& pool) :
    m_pool(pool), m_xfield(pool) {}

void import_pivot_data_fields::set_count(std::size_t count)
{
    m_current_fields.reserve(count);
}

iface::import_pivot_data_field* import_pivot_data_fields::start_data_field()
{
    m_xfield.reset([this](pivot_ref_data_field_t&& field) {
       m_current_fields.push_back(std::move(field));
    });
    return &m_xfield;
}

void import_pivot_data_fields::commit()
{
    m_func(std::move(m_current_fields));
}

void import_pivot_data_fields::reset(commit_func_type func)
{
    m_func = std::move(func);
    m_current_fields.clear();
}

void import_pivot_rc_item::set_repeat_items(std::size_t repeat)
{
    m_current.repeat = repeat;
}

void import_pivot_rc_item::set_item_type(pivot_field_item_t type)
{
    m_current.type = type;
}

void import_pivot_rc_item::set_data_item(std::size_t index)
{
    m_current.data_item = index;
}

void import_pivot_rc_item::append_index(std::size_t index)
{
    m_current.items.push_back(index);
}

void import_pivot_rc_item::commit()
{
    m_func(std::move(m_current));
}

void import_pivot_rc_item::reset(commit_func_type func)
{
    m_func = std::move(func);
    m_current = pivot_ref_rc_item_t{};
}

void import_pivot_rc_items::set_count(std::size_t count)
{
    m_current_rc_items.reserve(count);
}

iface::import_pivot_rc_item* import_pivot_rc_items::start_item()
{
    m_xitem.reset([this](pivot_ref_rc_item_t&& rc_item) {
        m_current_rc_items.push_back(std::move(rc_item));
    });
    return &m_xitem;
}

void import_pivot_rc_items::commit()
{
    m_func(std::move(m_current_rc_items));
}

void import_pivot_rc_items::reset(commit_func_type func)
{
    m_func = std::move(func);
    m_current_rc_items = pivot_ref_rc_items_t{};
}

import_pivot_table_def::import_pivot_table_def(document& doc) :
    m_doc(doc),
    m_current_pt(doc.get_string_pool()),
    m_data_fields(doc.get_string_pool())
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
    m_rc_fields.reset(
        pivot_axis_t::row,
        [this](pivot_ref_rc_fields_t&& fields) { m_current_pt.set_row_fields(std::move(fields)); }
    );

    return &m_rc_fields;
}

iface::import_pivot_rc_fields* import_pivot_table_def::start_column_fields()
{
    m_rc_fields.reset(
        pivot_axis_t::column,
        [this](pivot_ref_rc_fields_t&& fields) { m_current_pt.set_column_fields(std::move(fields)); }
    );
    return &m_rc_fields;
}

iface::import_pivot_page_fields* import_pivot_table_def::start_page_fields()
{
    m_page_fields.reset([this](pivot_ref_page_fields_t&& fields) {
        m_current_pt.set_page_fields(std::move(fields));
    });
    return &m_page_fields;
}

iface::import_pivot_data_fields* import_pivot_table_def::start_data_fields()
{
    m_data_fields.reset([this](pivot_ref_data_fields_t&& fields) {
        m_current_pt.set_data_fields(std::move(fields));
    });
    return &m_data_fields;
}

iface::import_pivot_rc_items* import_pivot_table_def::start_row_items()
{
    m_rc_items.reset([this](pivot_ref_rc_items_t&& rc_items) {
        m_current_pt.set_row_items(std::move(rc_items));
    });
    return &m_rc_items;
}

iface::import_pivot_rc_items* import_pivot_table_def::start_col_items()
{
    m_rc_items.reset([this](pivot_ref_rc_items_t&& rc_items) {
        m_current_pt.set_column_items(std::move(rc_items));
    });
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
