/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "xml_context_base.hpp"
#include "gnumeric_types.hpp"

#include <vector>

namespace orcus {

namespace spreadsheet { namespace iface {

class import_factory;

}}

class gnumeric_names_context : public xml_context_base
{
public:
    gnumeric_names_context(
        session_context& session_cxt, const tokens& tokens,
        spreadsheet::iface::import_factory* factory);

    virtual ~gnumeric_names_context() override;

    virtual void start_element(
        xmlns_id_t ns, xml_token_t name, const std::vector<xml_token_attr_t>& attrs) override;
    virtual bool end_element(xmlns_id_t ns, xml_token_t name) override;
    virtual void characters(std::string_view str, bool transient) override;

    void reset();

    const std::vector<gnumeric_named_exp>& get_names() const;

private:
    spreadsheet::iface::import_factory* mp_factory = nullptr;
    std::vector<gnumeric_named_exp> m_named_exps;
    gnumeric_named_exp m_current_name;
};

} // namespace orcus

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
