/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_ORCUS_SPREADSHEET_CSV_DUMPER_HPP
#define INCLUDED_ORCUS_SPREADSHEET_CSV_DUMPER_HPP

#include <string>
#include <ixion/types.hpp>

namespace orcus { namespace spreadsheet {

class document;

namespace detail {

class csv_dumper
{
    const document& m_doc;
    const char m_sep;
    const char m_quote;

public:
    csv_dumper(const document& doc);

    void dump(const std::string& filepath, ixion::sheet_t sheet_id) const;
};

}}}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

