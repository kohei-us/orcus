/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_ORCUS_SPREADSHEET_IMPORT_INTERFACE_PIVOT_HPP
#define INCLUDED_ORCUS_SPREADSHEET_IMPORT_INTERFACE_PIVOT_HPP

#include <cstdlib>

#include "orcus/spreadsheet/types.hpp"
#include "orcus/types.hpp"
#include "orcus/env.hpp"

// NB: This header must not depend on ixion, as it needs to be usable for
// those clients that provide their own formula engine.  Other headers in
// the orcus::spreadsheet namespace may depend on ixion.

namespace orcus { namespace spreadsheet { namespace iface {

/**
 * Interface for importing pivot cache definition.
 */
class ORCUS_DLLPUBLIC import_pivot_cache_definition
{
public:
    virtual ~import_pivot_cache_definition() = 0;
};

}}}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
