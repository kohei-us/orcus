/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <orcus/spreadsheet/import_interface_pivot_table_def.hpp>
#include <orcus/spreadsheet/pivot.hpp>

#include <functional>

namespace orcus { namespace spreadsheet {

class document;

namespace detail {

class import_pivot_field : public iface::import_pivot_field
{
public:
    using commit_func_type = std::function<void(pivot_field_t&&)>;

    void set_item_count(std::size_t count) override;
    void set_axis(pivot_axis_t axis) override;
    void append_item(std::size_t index, bool hidden) override;
    void append_item(pivot_field_item_t type) override;
    void commit() override;

    void reset(commit_func_type func);

private:
    commit_func_type m_func;
    pivot_field_t m_current_field;
};

class import_pivot_fields : public iface::import_pivot_fields
{
public:
    using commit_func_type = std::function<void(pivot_fields_t&&)>;

    void set_count(std::size_t count) override;
    iface::import_pivot_field* start_pivot_field() override;
    void commit() override;

    void reset(commit_func_type func);

private:
    commit_func_type m_func;
    import_pivot_field m_xfield;
    pivot_fields_t m_current_fields;
};

class import_pivot_rc_fields : public iface::import_pivot_rc_fields
{
public:
    using commit_func_type = std::function<void(pivot_ref_rc_fields_t&&)>;

    virtual void set_count(std::size_t count) override;
    virtual void append_field(std::size_t index) override;
    virtual void append_data_field() override;
    virtual void commit() override;

    void reset(pivot_axis_t axis, commit_func_type func);

private:
    commit_func_type m_func;
    pivot_axis_t m_axis = pivot_axis_t::unknown;
    pivot_ref_rc_fields_t m_fields;
};

class import_pivot_page_field : public iface::import_pivot_page_field
{
public:
    using commit_func_type = std::function<void(pivot_ref_page_field_t&&)>;

    virtual void set_field(std::size_t index) override;
    virtual void set_item(std::size_t index) override;
    virtual void commit() override;

    void reset(commit_func_type func);

private:
    commit_func_type m_func;
    pivot_ref_page_field_t m_current_field;
};

class import_pivot_page_fields : public iface::import_pivot_page_fields
{
public:
    using commit_func_type = std::function<void(pivot_ref_page_fields_t&&)>;

    virtual void set_count(std::size_t count) override;
    virtual iface::import_pivot_page_field* start_page_field() override;
    virtual void commit() override;

    void reset(commit_func_type func);

private:
    commit_func_type m_func;
    pivot_ref_page_fields_t m_current_fields;
    import_pivot_page_field m_xfield;
};

class import_pivot_data_field : public iface::import_pivot_data_field
{
public:
    using commit_func_type = std::function<void(pivot_ref_data_field_t&&)>;

    import_pivot_data_field(string_pool& pool);

    virtual void set_field(std::size_t index) override;
    virtual void set_name(std::string_view name) override;
    virtual void set_subtotal_function(pivot_data_subtotal_t func) override;
    virtual void set_show_data_as(
        pivot_data_show_data_as_t type, std::size_t base_field, std::size_t base_item) override;
    virtual void commit() override;

    void reset(commit_func_type func);

private:
    string_pool& m_pool;
    commit_func_type m_func;
    pivot_ref_data_field_t m_current_field;
};

class import_pivot_data_fields : public iface::import_pivot_data_fields
{
public:
    using commit_func_type = std::function<void(pivot_ref_data_fields_t&&)>;

    import_pivot_data_fields(string_pool& pool);

    virtual void set_count(std::size_t count) override;
    virtual iface::import_pivot_data_field* start_data_field() override;
    virtual void commit() override;

    void reset(commit_func_type func);

private:
    string_pool& m_pool;
    commit_func_type m_func;
    pivot_ref_data_fields_t m_current_fields;
    import_pivot_data_field m_xfield;
};

class import_pivot_rc_item : public iface::import_pivot_rc_item
{
public:
    using commit_func_type = std::function<void(pivot_ref_rc_item_t&&)>;

    virtual void set_repeat_items(std::size_t repeat) override;
    virtual void set_item_type(pivot_field_item_t type) override;
    virtual void set_data_item(std::size_t index) override;
    virtual void append_index(std::size_t index) override;
    virtual void commit() override;

    void reset(commit_func_type func);

private:
    commit_func_type m_func;
    pivot_ref_rc_item_t m_current;
};

class import_pivot_rc_items : public iface::import_pivot_rc_items
{
public:
    using commit_func_type = std::function<void(pivot_ref_rc_items_t&&)>;

    virtual void set_count(std::size_t count) override;
    virtual iface::import_pivot_rc_item* start_item() override;
    virtual void commit() override;

    void reset(commit_func_type func);

private:
    import_pivot_rc_item m_xitem;
    commit_func_type m_func;
    pivot_ref_rc_items_t m_current_rc_items;
};

class import_pivot_table_def : public iface::import_pivot_table_definition
{
    document& m_doc;

    pivot_table m_current_pt;

    import_pivot_fields m_pivot_fields;
    import_pivot_rc_fields m_rc_fields;
    import_pivot_page_fields m_page_fields;
    import_pivot_data_fields m_data_fields;
    import_pivot_rc_items m_rc_items;

public:
    import_pivot_table_def(document& doc);

    virtual void set_name(std::string_view name) override;
    virtual void set_cache_id(pivot_cache_id_t cache_id) override;
    virtual void set_range(const range_t& ref) override;

    virtual iface::import_pivot_fields* start_pivot_fields() override;
    virtual iface::import_pivot_rc_fields* start_row_fields() override;
    virtual iface::import_pivot_rc_fields* start_column_fields() override;
    virtual iface::import_pivot_page_fields* start_page_fields() override;
    virtual iface::import_pivot_data_fields* start_data_fields() override;
    virtual iface::import_pivot_rc_items* start_row_items() override;
    virtual iface::import_pivot_rc_items* start_col_items() override;
    virtual void commit() override;

    void reset();
};

}}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
