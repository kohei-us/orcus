/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "sheet_impl.hpp"

#include <boost/filesystem.hpp>

namespace orcus { namespace spreadsheet {

class document;

namespace detail {

class debug_state_dumper
{
    const sheet_impl& m_sheet;
    std::string_view m_sheet_name;

public:
    debug_state_dumper(const sheet_impl& sheet, std::string_view sheet_name);

    void dump(const boost::filesystem::path& outdir) const;
};

}}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
