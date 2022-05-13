/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "debug_state_dumper.hpp"
#include "check_dumper.hpp"

namespace fs = boost::filesystem;

namespace orcus { namespace spreadsheet { namespace detail {

sheet_debug_state_dumper::sheet_debug_state_dumper(const sheet_impl& sheet, std::string_view sheet_name) :
    m_sheet(sheet), m_sheet_name(sheet_name) {}

void sheet_debug_state_dumper::dump(const fs::path& outdir) const
{
    dump_cell_values(outdir);
}

void sheet_debug_state_dumper::dump_cell_values(const fs::path& outdir) const
{
    check_dumper dumper{m_sheet, m_sheet_name};
    fs::path outpath = outdir / "cell-values.txt";
    std::ofstream of{outpath};
    if (of)
        dumper.dump(of);
}

}}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
