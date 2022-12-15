/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_ORCUS_SPREADSHEET_IMPORT_INTERFACE_PIVOT_HPP
#define INCLUDED_ORCUS_SPREADSHEET_IMPORT_INTERFACE_PIVOT_HPP

#include <cstdlib>

#include "types.hpp"
#include "../types.hpp"
#include "../env.hpp"

// NB: This header must not depend on ixion, as it needs to be usable for
// those clients that provide their own formula engine.  Other headers in
// the orcus::spreadsheet namespace may depend on ixion.

namespace orcus { namespace spreadsheet { namespace iface {

class import_pivot_cache_field_group;

/**
 * Interface for importing pivot cache definitions.
 */
class ORCUS_DLLPUBLIC import_pivot_cache_definition
{
public:
    virtual ~import_pivot_cache_definition();

    /**
     * Specify that the source data of this pivot cache is located on a local
     * worksheet.
     *
     * @param ref range string specifying the source range.
     * @param sheet_name name of the worksheet where the source data is located.
     *
     * @todo use the ref resolver to resolve the range.
     */
    virtual void set_worksheet_source(std::string_view ref, std::string_view sheet_name) = 0;

    /**
     * Specify that the source data of this pivot cache is associated with a
     * table.
     *
     * @param table_name name of the table.
     */
    virtual void set_worksheet_source(std::string_view table_name) = 0;

    /**
     * Set the total number of fields present in this pivot cache.
     *
     * @param n total number of fields in this pivot cache.
     */
    virtual void set_field_count(size_t n) = 0;

    /**
     * Set the name of the field in the current field buffer.
     *
     * @param name field name.
     */
    virtual void set_field_name(std::string_view name) = 0;

    /**
     * Set the lowest value of the field in the current field buffer.
     *
     * @param v lowest value of the field.
     */
    virtual void set_field_min_value(double v) = 0;

    /**
     * Set the highest value of the field in the current field buffer.
     *
     * @param v highest value of the field.
     */
    virtual void set_field_max_value(double v) = 0;

    /**
     * Set the lowest date value of the field in the current field buffer.
     *
     * @param dt lowest date value of the field.
     */
    virtual void set_field_min_date(const date_time_t& dt) = 0;

    /**
     * Set the highest date value of the field in the current field buffer.
     *
     * @param dt highest date value of the field.
     */
    virtual void set_field_max_date(const date_time_t& dt) = 0;

    /**
     * Mark the current field as a group field and initiate its import.
     *
     * The implementor should create an internal storage to prepare for the
     * importing of field group data when this method gets called.
     *
     * @param base_index 0-based index of the field this field is the parent
     *                   group of.
     * @return interface for importing group field data, or a @p nullptr if the
     *         implementor doesn't support it.
     */
    virtual import_pivot_cache_field_group* start_field_group(size_t base_index) = 0;

    /**
     * Commit the field in the current field buffer to the pivot cache model.
     */
    virtual void commit_field() = 0;

    /**
     * Set a string value to the current field item buffer.
     *
     * @param value string value.
     */
    virtual void set_field_item_string(std::string_view value) = 0;

    /**
     * Set a numeric value to the current field item buffer.
     *
     * @param v numeric value.
     */
    virtual void set_field_item_numeric(double v) = 0;

    /**
     * Set a date-time value to the current field item buffer.
     *
     * @param dt date-time value.
     */
    virtual void set_field_item_date_time(const date_time_t& dt) = 0;

    /**
     * Set an error value to the current field item buffer,
     *
     * @param ev error value.
     */
    virtual void set_field_item_error(error_value_t ev) = 0;

    /**
     * Commit the field item in current field item buffer to the current field
     * model.
     */
    virtual void commit_field_item() = 0;

    /**
     * Commit the current pivot cache model to the document model.
     */
    virtual void commit() = 0;
};

/**
 * Interface for importing group field settings in a pivot cache definition.
 */
class ORCUS_DLLPUBLIC import_pivot_cache_field_group
{
public:
    virtual ~import_pivot_cache_field_group();

    /**
     * Establish a linkage between a base item to a group item.
     *
     * The index to corresponding base item is inferred from the order of this
     * method being called; the first call to this method implies a base item
     * index of 0, the second call implies an index of 1, and so on.
     *
     * This method is called only for a non-range group field; a group field
     * where parent-to-child item relationships are manually defined.
     *
     * @param group_item_index 0-based index for the group item.
     */
    virtual void link_base_to_group_items(size_t group_item_index) = 0;

    /**
     * Set an individual field item value that is of string type to the
     * current internal buffer.
     *
     * This method can be called either for a range group field or a non-range
     * one.
     *
     * @param value field item value.
     */
    virtual void set_field_item_string(std::string_view value) = 0;

    /**
     * Set an individual field item value that is of numeric type to the
     * current internal buffer.
     *
     * This method can be called either for a range group field or a non-range
     * one.
     *
     * @param v field item value.
     */
    virtual void set_field_item_numeric(double v) = 0;

    /**
     * Commit the current internal field item buffer to the group.
     */
    virtual void commit_field_item() = 0;

    /**
     * Set the range grouping type.
     *
     * The current group field implicitly becomes a range group field when
     * this method is called.
     *
     * @param group_by type of range grouping.
     */
    virtual void set_range_grouping_type(pivot_cache_group_by_t group_by) = 0;

    /**
     * Set whether the current range group field has an automatic start
     * position.
     *
     * The current group field implicitly becomes a range group field when
     * this method is called.
     *
     * @param b whether or not the current range group field has an automatic
     *          start position.
     */
    virtual void set_range_auto_start(bool b) = 0;

    /**
     * Set whether the current range group field has an automatic end
     * position.
     *
     * The current group field implicitly becomes a range group field when
     * this method is called.
     *
     * @param b whether or not the current range group field has an automatic
     *          end position.
     */
    virtual void set_range_auto_end(bool b) = 0;

    /**
     * Set the start number of the current range group field.
     *
     * The current group field implicitly becomes a range group field when
     * this method is called.
     *
     * @param v start number of the current range group field.
     */
    virtual void set_range_start_number(double v) = 0;

    /**
     * Set the end number of the current range group field.
     *
     * The current group field implicitly becomes a range group field when
     * this method is called.
     *
     * @param v end number of the current range group field.
     */
    virtual void set_range_end_number(double v) = 0;

    /**
     * Set the start date of the current range group field.
     *
     * The current group field implicitly becomes a range group field when
     * this method is called.
     *
     * @param dt start date of the current range group field.
     */
    virtual void set_range_start_date(const date_time_t& dt) = 0;

    /**
     * Set the end date of the current range group field.
     *
     * The current group field implicitly becomes a range group field when
     * this method is called.
     *
     * @param dt end date of the current range group field.
     */
    virtual void set_range_end_date(const date_time_t& dt) = 0;

    /**
     * Set the interval of the current range group field.  If the current
     * range is a date range, the value represents the number of days.
     *
     * @param v interval of the current range group field.
     */
    virtual void set_range_interval(double v) = 0;

    /**
     * Commit the current field group data to the parent field.
     */
    virtual void commit() = 0;
};

/**
 * Interface for importing pivot cache records.
 */
class ORCUS_DLLPUBLIC import_pivot_cache_records
{
public:
    virtual ~import_pivot_cache_records();

    /**
     * Set the number of records included in pivot cache records.
     *
     * @note This method gets called before the very first record gets imported.
     *       The implementor can use this call as an opportunity to initialize
     *       any internal buffers used to store the imported records.
     *
     * @param n number of records included in pivot cache records.
     */
    virtual void set_record_count(size_t n) = 0;

    /**
     * Append to the current record buffer a numeric value as a column value.
     *
     * @param v numeric value to append to the current record buffer as a column
     *          value.
     */
    virtual void append_record_value_numeric(double v) = 0;

    /**
     * Append to the current record buffer a character value as a column value.
     *
     * @param s character value to append to the current record buffer as a
     *          column value.
     */
    virtual void append_record_value_character(std::string_view s) = 0;

    /**
     * Append to the current record buffer a column value referenced by an index
     * into the shared items table of a pivot cache field.  The corresponding
     * field in the pivot cache definition should provide the shared items table
     * that this index references.
     *
     * @param index index into the shared items table of a pivot cache field.
     */
    virtual void append_record_value_shared_item(size_t index) = 0;

    /**
     * Commit the record in the current record buffer.
     *
     * The implementor can clear the buffer afterward.
     */
    virtual void commit_record() = 0;

    /**
     * Commit the entire records set to the document store.
     */
    virtual void commit() = 0;
};

}}}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
