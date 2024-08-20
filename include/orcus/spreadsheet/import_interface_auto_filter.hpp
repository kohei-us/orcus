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

class ORCUS_DLLPUBLIC import_auto_filter_node
{
public:
    virtual ~import_auto_filter_node();

    virtual void append_item(auto_filter_op_t op, double value) = 0;
    virtual void append_item(auto_filter_op_t op, std::string_view value) = 0;
    virtual import_auto_filter_node* append_item(auto_filter_node_op_t op) = 0;

    virtual void commit() = 0;
};

class ORCUS_DLLPUBLIC import_auto_filter
{
public:
    virtual ~import_auto_filter();

    /**
     * Signal the start of the iport of a set of auto-filter rules associated
     * with a single column.
     *
     * @param col    0-based offset position of the field relative to the
     *               left-most column of the filtered range.
     * @param op     Boolean operator connecting the multiple filter rules at
     *               the root level of the filter rules tree.
     *
     * @return Interface
     *
     * @note Note that the import_auto_filter implementer <i>must</i> return a
     * non-null pointer.
     */
    virtual iface::import_auto_filter_node* start_column(col_t col, auto_filter_node_op_t op) = 0;

    /**
     * Commit all the auto filter data stored in the buffer so far to the
     * destination store.
     */
    virtual void commit() = 0;
};

namespace old {

/**
 * Interface for importing auto filters.
 *
 * Importing a single auto filter would roughly follow the following flow:
 *
 * @code{.cpp}
 * import_auto_filter* iface = ... ;
 *
 * range_t range;
 * range.first.column = 0;
 * range.first.row = 0;
 * range.last.column = 3;
 * range.last.row = 1000;
 * iface->set_range(range); // Auto filter is applied for A1:D1001.
 *
 * // Column A is filtered for a value of "A".
 * iface->set_column(0);
 * iface->append_column_match_value("A");
 * iface->commit_column();
 *
 * // Column D is filtered for values of 1 and 4.
 * iface->set_column(3);
 * iface->append_column_match_value("1");
 * iface->append_column_match_value("4");
 * iface->commit_column();
 *
 * // Push the autofilter data in the current buffer to the sheet store.
 * iface->commit();
 * @endcode
 */
class ORCUS_DLLPUBLIC import_auto_filter
{
public:
    virtual ~import_auto_filter();

    /**
     * Specify the range where the auto filter is applied.
     *
     * @param range structure containing the top-left and bottom-right
     *              positions of the auto filter range.
     */
    virtual void set_range(const range_t& range) = 0;

    /**
     * Specify the column position of a filter. The position is relative to
     * the first column in the auto filter range.  This method gets called at
     * the beginning of each column filter data.  The implementor may initialize
     * the column filter data buffer when this method is called.
     *
     * @note This column position is relative to the first column in the
     *       autofilter range.
     *
     * @param col 0-based column position of a filter relative to the first
     *            column of the auto filter range.
     */
    virtual void set_column(col_t col) = 0;

    /**
     * Append a match value to the current column filter.  A single column
     * filter may have one or more match values.
     *
     * @param value match value to append to the current column filter.
     */
    virtual void append_column_match_value(std::string_view value) = 0;

    /**
     * Commit the current column filter data to the current auto filter buffer.
     * The implementor may clear the current column filter buffer after this
     * call.
     */
    virtual void commit_column() = 0;

    /**
     * Commit current auto filter data stored in the buffer to the sheet store.
     */
    virtual void commit() = 0;
};

} // namespace old

}}}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
