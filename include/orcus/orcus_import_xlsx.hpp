/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "interface.hpp"

namespace orcus {

namespace spreadsheet { namespace iface {
    class import_table;
    class import_reference_resolver;
}}

class ORCUS_DLLPUBLIC import_xlsx
{
public:
    import_xlsx() = delete;
    import_xlsx(const import_xlsx&) = delete;
    import_xlsx& operator=(const import_xlsx&) = delete;

    static void read_table(
        std::string_view s,
        spreadsheet::iface::import_table& table,
        spreadsheet::iface::import_reference_resolver& resolver);
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
