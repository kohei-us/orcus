/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_ORCUS_SPREADSHEET_IMPORT_INTERFACE_PIVOT_HPP
#define INCLUDED_ORCUS_SPREADSHEET_IMPORT_INTERFACE_PIVOT_HPP

#include <cstdlib>

#include "orcus/spreadsheet/types.hpp"
#include "orcus/types.hpp"
#include "orcus/env.hpp"

// NB: This header must not depend on ixion, as it needs to be usable for
// those clients that provide their own formula engine.  Other headers in
// the orcus::spreadsheet namespace may depend on ixion.

namespace orcus { namespace spreadsheet { namespace iface {

class import_pivot_cache_field_group;

/**
 * Interface for importing pivot cache definition.
 */
class ORCUS_DLLPUBLIC import_pivot_cache_definition
{
public:
    virtual ~import_pivot_cache_definition();

    virtual void set_worksheet_source(
        const char* ref, size_t n_ref, const char* sheet_name, size_t n_sheet_name) = 0;

    virtual void set_field_count(size_t n) = 0;

    virtual void set_field_name(const char* p, size_t n) = 0;

    virtual void set_field_min_value(double v) = 0;

    virtual void set_field_max_value(double v) = 0;

    virtual void set_field_min_date(const date_time_t& d) = 0;

    virtual void set_field_max_date(const date_time_t& d) = 0;

    /**
     * Mark the current field as a group field.
     *
     * This method gets called first to signify that the current field is a
     * group field.
     *
     * @param base_index 0-based index of the field this field is the parent
     *                   group of.
     * @return interface for importing group field data.
     */
    virtual import_pivot_cache_field_group* create_field_group(size_t base_index) = 0;

    virtual void commit_field() = 0;

    virtual void set_field_item_string(const char* p, size_t n) = 0;

    virtual void set_field_item_numeric(double v) = 0;

    virtual void commit_field_item() = 0;

    virtual void commit() = 0;
};

class ORCUS_DLLPUBLIC import_pivot_cache_field_group
{
public:
    virtual ~import_pivot_cache_field_group();

    /**
     * Establish a linkage between a base item to a gruop item.
     *
     * The index to corresponding base item is inferred from the order of this
     * method being called; the first call to this method implies a base item
     * index of 0, the second call implies an index of 1, and so on.
     *
     * @param group_item_index 0-based index for the group item.
     */
    virtual void link_base_to_group_items(size_t group_item_index) = 0;

    virtual void set_field_item_string(const char* p, size_t n) = 0;

    virtual void set_field_item_numeric(double v) = 0;

    virtual void commit_field_item() = 0;

    virtual void set_auto_start(bool b) = 0;

    virtual void set_auto_end(bool b) = 0;

    virtual void set_start_number(double v) = 0;

    virtual void set_end_number(double v) = 0;

    virtual void set_group_interval(double v) = 0;

    /**
     * Commit the current field group data to the parent field.
     */
    virtual void commit() = 0;
};

}}}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
