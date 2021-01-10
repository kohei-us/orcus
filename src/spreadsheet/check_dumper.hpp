/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_ORCUS_SPREADSHEET_CHECK_DUMPER_HPP
#define INCLUDED_ORCUS_SPREADSHEET_CHECK_DUMPER_HPP

#include <ostream>

#include "orcus/pstring.hpp"

namespace orcus { namespace spreadsheet {

struct sheet_impl;

namespace detail {

class check_dumper
{
    const sheet_impl& m_sheet;
    pstring m_sheet_name;

public:
    check_dumper(const sheet_impl& sheet, const pstring& sheet_name);
    void dump(std::ostream& os) const;

private:
    void dump_cell_values(std::ostream& os) const;
    void dump_merged_cell_info(std::ostream& os) const;
};

}}}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
