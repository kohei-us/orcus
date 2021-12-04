/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "xml_context_base.hpp"

namespace orcus {

/**
 * This context only tracks the scopes of all encountered elements but does
 * nothing else.  It is to be used when you need to ignore a whole sub
 * structure of an XML document.
 */
class xml_empty_context : public xml_context_base
{
public:
    xml_empty_context(session_context& session_cxt, const tokens& tokens);

    virtual xml_context_base* create_child_context(xmlns_id_t ns, xml_token_t name) override;

    virtual void end_child_context(
        xmlns_id_t ns, xml_token_t name, xml_context_base* child) override;

    virtual void start_element(
        xmlns_id_t ns, xml_token_t name, const std::vector<xml_token_attr_t>& attrs) override;

    virtual bool end_element(xmlns_id_t ns, xml_token_t name) override;

    virtual void characters(std::string_view str, bool transient) override;
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
