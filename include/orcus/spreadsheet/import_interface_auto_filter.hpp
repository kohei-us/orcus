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
 * Interface for importing a set of multiple filter values.
 */
class ORCUS_DLLPUBLIC import_auto_filter_multi_values
{
public:
    virtual ~import_auto_filter_multi_values();

    /**
     * Add a single filter value to the current buffer.
     *
     * @param value Filter value to add to the current buffer.
     */
    virtual void add_value(std::string_view value) = 0;

    /**
     * Commit the values stored in the current buffer to the destination store.
                                                                                */
    virtual void commit() = 0;
};

/**
 * Interface for importing a single node in a larger auto-filter structure.
 *
 * Note that one auto-filter structure may consist of nested filter nodes.
 */
class ORCUS_DLLPUBLIC import_auto_filter_node
{
public:
    virtual ~import_auto_filter_node();

    /**
     * Append to this node a new filter item with a numeric value.
     *
     * @param field  0-based field index which is the offset from the left-most
     *               column of the filtered range.
     * @param op     Operator for the filter item.
     * @param value  Numeric value associated with the operator for the filter
     *               item.  Note that some operators may not require a value.
     */
    virtual void append_item(col_t field, auto_filter_op_t op, double value) = 0;

    /**
     * Append to this node a new filter item with a string value.
     *
     * Note that the the life cycle of the string value passed to this call is
     * only guaranteed to persist during the call.
     *
     * @param field  0-based field index which is the offset from the left-most
     *               column of the filtered range.
     * @param op     Operator for the filter item.
     * @param value  String value associated with the operator for the filter
     *               item.  Note that some operators may not require a value.
     * @param regex  Whether or not the string value should be interpreted as a
     *               regular expression.
     */
    virtual void append_item(col_t field, auto_filter_op_t op, std::string_view value, bool regex) = 0;

    /**
     * Append to this node a new filter item with no associated value.
     *
     * @param field  0-based field index which is the offset from the left-most
     *               column of the filtered range.
     * @param op     Operator for the filter item.
     */
    virtual void append_item(col_t field, auto_filter_op_t op) = 0;

    /**
     * Start a new node of filter rules as a filter item to this node. The new
     * node should be appended to this node as new filter item when it is
     * committed.
     *
     * @param op Operator to use to link the items stored in the new node.
     *
     * @return Interface for importing the filter rules for the new node.
     *
     * @note Note that the import_auto_filter_node implementer
     *       <i>must</i> return a non-null pointer.
     */
    virtual import_auto_filter_node* start_node(auto_filter_node_op_t op) = 0;

    /**
     * Start importing a set of multiple filter values.  Note that a set of
     * multiple filter values and individual filter items cannot co-exist in the
     * same filter node.
     *
     * @return Interface for importing a set of multiple filter values for the
     *         current filter node.
     *
     * @note Note that the import_auto_filter_node implementer
     *       <i>must</i> return a non-null pointer.
     */
    virtual import_auto_filter_multi_values* start_multi_values(col_t field) = 0;

    /**
     * Commit the filter node data stored in the buffer to the destination
     * store.
     */
    virtual void commit() = 0;
};

/**
 * Interface for importing auto filters.
 */
class ORCUS_DLLPUBLIC import_auto_filter
{
public:
    virtual ~import_auto_filter();

    /**
     * Signal the start of the import of a set of auto-filter rules associated
     * with a single column.
     *
     * @param col_offset 0-based offset position of the field relative to the
     *                   left-most column of the filtered range.
     * @param op         Boolean operator connecting the multiple filter rules
     *                   at the root level of the filter rules tree.
     *
     * @return Interface for importing the root node of the auto-filter rules
     *         for a column.
     *
     * @note Note that the import_auto_filter implementer <i>must</i> return a
     * non-null pointer.
     */
    virtual iface::import_auto_filter_node* start_node(auto_filter_node_op_t op) = 0;

    /**
     * Commit all the auto filter data stored in the buffer so far to the
     * destination store.
     */
    virtual void commit() = 0;
};

}}}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
