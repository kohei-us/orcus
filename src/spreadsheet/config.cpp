/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "orcus/spreadsheet/config.hpp"

namespace orcus { namespace spreadsheet {

document_config::document_config() :
    output_precision(-1) {}

document_config::document_config(const document_config& r) :
    output_precision(r.output_precision) {}

document_config::~document_config() {}

document_config& document_config::operator= (const document_config& r)
{
    output_precision = r.output_precision;
    return *this;
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
