/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_ORCUS_SPREADSHEET_FACTORY_PIVOT_HPP
#define INCLUDED_ORCUS_SPREADSHEET_FACTORY_PIVOT_HPP

#include "orcus/spreadsheet/pivot.hpp"
#include "orcus/spreadsheet/document.hpp"
#include "orcus/spreadsheet/import_interface_pivot.hpp"

#include <ixion/formula_name_resolver.hpp>

namespace orcus { namespace spreadsheet {

class import_pc_field_group;

/**
 * Concrete implementation of the import_pivot_cache_definition interface.
 */
class import_pivot_cache_def : public iface::import_pivot_cache_definition
{
    enum source_type { unknown = 0, worksheet, external, consolidation, scenario };

    document& m_doc;

    pivot_cache_id_t m_cache_id = 0;

    source_type m_src_type = unknown;
    pstring m_src_sheet_name;
    ixion::abs_range_t m_src_range;

    std::unique_ptr<pivot_cache> m_cache;
    pivot_cache::fields_type m_current_fields;
    pivot_cache_field_t m_current_field;
    pivot_cache_item_t m_current_field_item;

    std::unique_ptr<import_pc_field_group> m_current_field_group;

private:
    pstring intern(const char* p, size_t n);

public:
    import_pivot_cache_def(document& doc);
    ~import_pivot_cache_def();

    void create_cache(pivot_cache_id_t cache_id);

    virtual void set_worksheet_source(
        const char* ref, size_t n_ref, const char* sheet_name, size_t n_sheet_name) override;

    virtual void set_field_count(size_t n) override;

    virtual void set_field_name(const char* p, size_t n) override;

    virtual iface::import_pivot_cache_field_group* create_field_group(size_t base_index) override;

    virtual void set_field_min_value(double v) override;

    virtual void set_field_max_value(double v) override;

    virtual void set_field_min_date(const date_time_t& dt) override;

    virtual void set_field_max_date(const date_time_t& dt) override;

    virtual void commit_field() override;

    virtual void set_field_item_string(const char* p, size_t n) override;

    virtual void set_field_item_numeric(double v) override;

    virtual void set_field_item_date_time(const date_time_t& dt) override;

    virtual void commit_field_item() override;

    virtual void commit() override;
};

/**
 * Concrete implementation of the import_pivot_cache_records interface. 
 */
class import_pivot_cache_records : public iface::import_pivot_cache_records
{
    document& m_doc;
    pivot_cache* m_cache; //< cache to push the records to at the very end.

    pivot_cache_record_t m_current_record;
    pivot_cache::records_type m_records;

public:
    import_pivot_cache_records(document& doc);
    ~import_pivot_cache_records();

    void set_cache(pivot_cache* p);

    virtual void set_record_count(size_t n) override;

    virtual void append_record_value_numeric(double v) override;

    virtual void append_record_value_character(const char* p, size_t n) override;

    virtual void append_record_value_shared_item(size_t index) override;

    virtual void commit_record() override;

    virtual void commit() override;
};

}}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
