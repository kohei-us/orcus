/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_ODS_DDE_LINKS_CONTEXT_HPP
#define INCLUDED_ODS_DDE_LINKS_CONTEXT_HPP

#include "xml_context_base.hpp"

namespace orcus {

/**
 * Handle <table:dde-links> element structure.  For now, this context exists
 * only to ignore all DDE related data.
 */
class ods_dde_links_context : public xml_context_base
{
public:
    ods_dde_links_context(session_context& session_cxt, const tokens& tokens);
    virtual ~ods_dde_links_context();

    virtual bool can_handle_element(xmlns_id_t ns, xml_token_t name) const override;

    virtual xml_context_base* create_child_context(xmlns_id_t ns, xml_token_t name) override;

    virtual void end_child_context(xmlns_id_t ns, xml_token_t name, xml_context_base *child) override;

    virtual void start_element(xmlns_id_t ns, xml_token_t name, const::std::vector<xml_token_attr_t> &attrs) override;

    virtual bool end_element(xmlns_id_t ns, xml_token_t name) override;

    virtual void characters(std::string_view str, bool transient) override;

    void reset();
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
