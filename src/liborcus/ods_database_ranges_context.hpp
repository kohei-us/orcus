/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "xml_context_base.hpp"

namespace orcus {

class ods_database_ranges_context : public xml_context_base
{
public:
    ods_database_ranges_context(session_context& session_cxt, const tokens& tokens);
    virtual ~ods_database_ranges_context() override;

    virtual void start_element(xmlns_id_t ns, xml_token_t name, const xml_token_attrs_t& attrs) override;
    virtual bool end_element(xmlns_id_t ns, xml_token_t name) override;

    void reset();
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
