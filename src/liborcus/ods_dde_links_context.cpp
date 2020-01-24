/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "ods_dde_links_context.hpp"

namespace orcus {

ods_dde_links_context::ods_dde_links_context(session_context& session_cxt, const tokens& tokens) :
    xml_context_base(session_cxt, tokens) {}

ods_dde_links_context::~ods_dde_links_context() {}

bool ods_dde_links_context::can_handle_element(xmlns_id_t ns, xml_token_t name) const
{
    return true;
}

xml_context_base* ods_dde_links_context::create_child_context(xmlns_id_t ns, xml_token_t name)
{
    return nullptr;
}

void ods_dde_links_context::end_child_context(xmlns_id_t ns, xml_token_t name, xml_context_base *child)
{
}

void ods_dde_links_context::start_element(xmlns_id_t ns, xml_token_t name, const::std::vector<xml_token_attr_t> &attrs)
{
    xml_token_pair_t parent = push_stack(ns, name);
    (void)parent;

    warn_unhandled();
}

bool ods_dde_links_context::end_element(xmlns_id_t ns, xml_token_t name)
{
    return pop_stack(ns, name);
}

void ods_dde_links_context::characters(const pstring &str, bool transient)
{
}

void ods_dde_links_context::reset()
{
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
