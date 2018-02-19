/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_ORCUS_SPREADSHEET_CONFIG_HPP
#define INCLUDED_ORCUS_SPREADSHEET_CONFIG_HPP

#include "orcus/env.hpp"

#include <cstdint>

namespace orcus { namespace spreadsheet {

struct ORCUS_SPM_DLLPUBLIC document_config
{
    /**
     * Precision to use when converting numeric values to their string
     * representations.  A negative value indicates the precision is not being
     * specified.
     */
    int8_t output_precision;

    document_config();
    document_config(const document_config& r);
    ~document_config();

    document_config& operator= (const document_config& r);
};

}}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
