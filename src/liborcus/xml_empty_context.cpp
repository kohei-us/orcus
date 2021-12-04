/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "xml_empty_context.hpp"

namespace orcus {

xml_empty_context::xml_empty_context(session_context& session_cxt, const tokens& tokens) :
    xml_context_base(session_cxt, tokens)
{
}

xml_context_base* xml_empty_context::create_child_context(xmlns_id_t /*ns*/, xml_token_t /*name*/)
{
    return nullptr;
}

void xml_empty_context::end_child_context(
    xmlns_id_t /*ns*/, xml_token_t /*name*/, xml_context_base* /*child*/)
{
}

void xml_empty_context::start_element(
    xmlns_id_t ns, xml_token_t name, const::std::vector<xml_token_attr_t>& /*attrs*/)
{
    push_stack(ns, name);
}

bool xml_empty_context::end_element(xmlns_id_t ns, xml_token_t name)
{
    return pop_stack(ns, name);
}

void xml_empty_context::characters(std::string_view /*str*/, bool /*transient*/)
{
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
