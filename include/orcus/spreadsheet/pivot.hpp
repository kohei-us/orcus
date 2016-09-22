/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_ORCUS_SPREADSHEET_PIVOT_HPP
#define INCLUDED_ORCUS_SPREADSHEET_PIVOT_HPP

#include "orcus/spreadsheet/import_interface_pivot.hpp"
#include "orcus/env.hpp"

namespace orcus { namespace spreadsheet {

class ORCUS_SPM_DLLPUBLIC pivot_cache : public iface::import_pivot_cache_definition
{
public:
    pivot_cache();
    virtual ~pivot_cache();
};

}}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
