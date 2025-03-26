/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <orcus/spreadsheet/import_interface_pivot_table_def.hpp>

namespace orcus { namespace spreadsheet {

class import_pivot_field : public iface::import_pivot_field
{
public:
    void set_item_count(std::size_t count) override;
    void append_item(std::size_t index) override;
    void append_item(pivot_field_item_t type) override;
    void commit() override;
};

class import_pivot_fields : public iface::import_pivot_fields
{
    import_pivot_field m_field;

public:
    void set_count(std::size_t count) override;
    iface::import_pivot_field* start_pivot_field() override;
    void commit() override;
};

class import_pivot_rc_fields : public iface::import_pivot_rc_fields
{
public:
    virtual void set_count(std::size_t count) override;
    virtual void append_field(std::size_t index) override;
    virtual void commit() override;
};

class import_pivot_page_field : public iface::import_pivot_page_field
{
public:
    virtual void set_field(std::size_t index) override;
    virtual void commit() override;
};

class import_pivot_page_fields : public iface::import_pivot_page_fields
{
    import_pivot_page_field m_field;

public:
    virtual iface::import_pivot_page_field* start_page_field() override;
    virtual void commit() override;
};

class import_pivot_data_field : public iface::import_pivot_data_field
{
public:
    virtual void set_field(std::size_t index) override;
    virtual void commit() override;
};

class import_pivot_data_fields : public iface::import_pivot_data_fields
{
    import_pivot_data_field m_field;

public:
    virtual iface::import_pivot_data_field* start_data_field() override;
    virtual void commit() override;
};

class import_pivot_rc_item : public iface::import_pivot_rc_item
{
public:
    virtual void append_index(std::size_t index) override;
    virtual void commit() override;
};

class import_pivot_rc_items : public iface::import_pivot_rc_items
{
    import_pivot_rc_item m_item;

public:
    virtual iface::import_pivot_rc_item* start_item() override;
    virtual void commit() override;
};

class import_pivot_table_def : public iface::import_pivot_table_definition
{
    import_pivot_rc_fields m_rc_fields;
    import_pivot_page_fields m_page_fields;
    import_pivot_data_fields m_data_fields;
    import_pivot_rc_items m_rc_items;

public:
    virtual void set_name(std::string_view name) override;
    virtual void set_cache_id(pivot_cache_id_t cache_id) override;
    virtual void set_range(const range_t& ref) override;

    virtual iface::import_pivot_fields* start_pivot_fields() override;
    virtual iface::import_pivot_rc_fields* start_row_fields() override;
    virtual iface::import_pivot_rc_fields* start_col_fields() override;
    virtual iface::import_pivot_page_fields* start_page_fields() override;
    virtual iface::import_pivot_data_fields* start_data_fields() override;
    virtual iface::import_pivot_rc_items* start_row_items() override;
    virtual iface::import_pivot_rc_items* start_col_items() override;
};

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
