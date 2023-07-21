/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "gnumeric_styles_context.hpp"

namespace ss = orcus::spreadsheet;

namespace orcus {

gnumeric_styles_context::gnumeric_styles_context(
    session_context& session_cxt, const tokens& tokens,
    ss::iface::import_factory* factory) :
    xml_context_base(session_cxt, tokens),
    mp_factory(factory)
{
}

gnumeric_styles_context::~gnumeric_styles_context() = default;

void gnumeric_styles_context::start_element(
    xmlns_id_t ns, xml_token_t name, const std::vector<xml_token_attr_t>& attrs)
{
    push_stack(ns, name);
    (void)attrs;
    warn_unhandled();
}

bool gnumeric_styles_context::end_element(xmlns_id_t ns, xml_token_t name)
{
    return pop_stack(ns, name);
}

void gnumeric_styles_context::reset()
{
}

} // namespace orcus

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
