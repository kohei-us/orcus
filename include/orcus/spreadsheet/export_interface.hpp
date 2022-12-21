/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_ORCUS_SPREADSHEET_EXPORT_INTERFACE_HPP
#define INCLUDED_ORCUS_SPREADSHEET_EXPORT_INTERFACE_HPP

#include "types.hpp"
#include "../env.hpp"

#include <ostream>

namespace orcus { namespace spreadsheet { namespace iface {

/**
 * Interface for exporting sheet contents.
 */
class export_sheet
{
public:
    ORCUS_DLLPUBLIC virtual ~export_sheet() = 0;

    /**
     * Write the content of a cell to an output stream.
     *
     * @param os output stream to write the cell content to.
     * @param row 0-based row position of a cell.
     * @param col 0-based column position of a cell.
     */
    virtual void write_string(std::ostream& os, orcus::spreadsheet::row_t row, orcus::spreadsheet::col_t col) const = 0;
};

/**
 * Entry-point interface for exporting document contents.
 */
class export_factory
{
public:
    ORCUS_DLLPUBLIC virtual ~export_factory() = 0;

    /**
     * Obtain an interface for exporting sheet content.
     *
     * @param sheet_name name of the sheet to export.
     *
     * @return pointer to an interface for exporting sheet content.
     */
    virtual const export_sheet* get_sheet(std::string_view sheet_name) const = 0;
};

}}}




#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
