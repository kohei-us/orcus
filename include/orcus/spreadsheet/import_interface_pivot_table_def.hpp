/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "types.hpp"
#include "../types.hpp"
#include "../env.hpp"

// NB: This header must not depend on ixion, as it needs to be usable for
// those clients that provide their own formula engine.  Other headers in
// the orcus::spreadsheet namespace may depend on ixion.

namespace orcus { namespace spreadsheet { namespace iface {

/**
 * Interface for importing a pivot field as part of a pivot table.
 *
 * A pivot field is a single field that can be added to row, column or page
 * axes.
 */
class ORCUS_DLLPUBLIC import_pivot_field
{
public:
    virtual ~import_pivot_field();

    virtual void set_item_count(std::size_t count) = 0;

    virtual void set_axis(pivot_axis_t axis) = 0;

    /**
     * Append a pivot field item with an index.
     *
     * @param index Index into the corresponding cache field in the pivot cache.
     * @param hidden Whether or not this item is hidden.
     */
    virtual void append_item(std::size_t index, bool hidden) = 0;

    virtual void append_item(pivot_field_item_t type) = 0;

    virtual void commit() = 0;
};

class ORCUS_DLLPUBLIC import_pivot_fields
{
public:
    virtual ~import_pivot_fields();

    virtual void set_count(std::size_t count) = 0;

    virtual import_pivot_field* start_pivot_field() = 0;

    virtual void commit() = 0;
};

/**
 * Interface for importing either row or column fields.
 */
class ORCUS_DLLPUBLIC import_pivot_rc_fields
{
public:
    virtual ~import_pivot_rc_fields();

    virtual void set_count(std::size_t count) = 0;

    virtual void append_field(std::size_t index) = 0;

    /**
     * Append a special field that displays a list of data fields when the pivot
     * table contains more than one data field.
     */
    virtual void append_data_field() = 0;

    virtual void commit() = 0;
};

class ORCUS_DLLPUBLIC import_pivot_page_field
{
public:
    virtual ~import_pivot_page_field();

    virtual void set_field(std::size_t index) = 0;

    virtual void set_item(std::size_t index) = 0;

    virtual void commit() = 0;
};

class ORCUS_DLLPUBLIC import_pivot_page_fields
{
public:
    virtual ~import_pivot_page_fields();

    virtual void set_count(std::size_t count) = 0;

    virtual import_pivot_page_field* start_page_field() = 0;

    virtual void commit() = 0;
};

class ORCUS_DLLPUBLIC import_pivot_data_field
{
public:
    virtual ~import_pivot_data_field();

    virtual void set_field(std::size_t index) = 0;

    virtual void set_name(std::string_view name) = 0;

    virtual void set_subtotal_function(pivot_data_subtotal_t func) = 0;

    virtual void set_show_data_as(
        pivot_data_show_data_as_t type, std::size_t base_field, std::size_t base_item) = 0;

    virtual void commit() = 0;
};

class ORCUS_DLLPUBLIC import_pivot_data_fields
{
public:
    virtual ~import_pivot_data_fields();

    virtual void set_count(std::size_t count) = 0;

    virtual import_pivot_data_field* start_data_field() = 0;

    virtual void commit() = 0;
};

/**
 * Interface for importing a single row or column item.
 *
 * A single row or column item consists of a series of labels displayed in a
 * pivot table output. The labels are represented by their respective indexes
 * into the corresponding shared items in pivot cache definition.  The number
 * of indexes stored in one item is equal to the number of the corresponding
 * row or column fields.
 */
class ORCUS_DLLPUBLIC import_pivot_rc_item
{
public:
    virtual ~import_pivot_rc_item();

    /**
     * Set the number of items to be repeated from the previous row or column of
     * items.  This typically corresponds with the number of empty cells to fill
     * before the first non-empty item label to appear.
     *
     * @param repeat Number of items to repeat from the previous row or column,
     *               which typically is the number of empty cells to fill.
     */
    virtual void set_repeat_items(std::size_t repeat) = 0;

    /**
     * Set the type of items in the current row or column of items.
     *
     * @param type Item type in the current row or column of items.
     */
    virtual void set_item_type(pivot_field_item_t type) = 0;

    /**
     * Set the index of a data item in a data field with multiple data items.
     *
     * @param index Index of a referenced data item.
     */
    virtual void set_data_item(std::size_t index) = 0;

    /**
     * Append the index of an item of a field.
     *
     * @param index Index of a label.
     */
    virtual void append_index(std::size_t index) = 0;

    virtual void commit() = 0;
};

/**
 * Interface for importing a series of row or column items.
 */
class ORCUS_DLLPUBLIC import_pivot_rc_items
{
public:
    virtual ~import_pivot_rc_items();

    virtual void set_count(std::size_t count) = 0;

    virtual import_pivot_rc_item* start_item() = 0;

    virtual void commit() = 0;
};

/**
 * Interface for importing pivot table definitions.
 */
class ORCUS_DLLPUBLIC import_pivot_table_definition
{
public:
    virtual ~import_pivot_table_definition();

    virtual void set_name(std::string_view name) = 0;

    virtual void set_cache_id(pivot_cache_id_t cache_id) = 0;

    /**
     * Set the range of a pivot table.
     *
     * @param ref Range of a pivot table.
     */
    virtual void set_range(const range_t& ref) = 0;

    virtual import_pivot_fields* start_pivot_fields() = 0;

    virtual import_pivot_rc_fields* start_row_fields() = 0;

    virtual import_pivot_rc_fields* start_column_fields() = 0;

    virtual import_pivot_page_fields* start_page_fields() = 0;

    virtual import_pivot_data_fields* start_data_fields() = 0;

    virtual import_pivot_rc_items* start_row_items() = 0;

    virtual import_pivot_rc_items* start_col_items() = 0;

    virtual void commit() = 0;
};

}}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
