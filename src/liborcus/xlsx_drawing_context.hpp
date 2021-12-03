/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_ORCUS_XLSX_DRAWING_CONTEXT_HPP
#define INCLUDED_ORCUS_XLSX_DRAWING_CONTEXT_HPP

#include "xml_context_base.hpp"

namespace orcus {

struct session_context;
class tokens;

class xlsx_drawing_context : public xml_context_base
{
    long m_col;
    long m_row;
    long m_col_offset;
    long m_row_offset;

public:
    xlsx_drawing_context(session_context& cxt, const tokens& tkns);

    virtual ~xlsx_drawing_context();

    virtual xml_context_base* create_child_context(xmlns_id_t ns, xml_token_t name);

    virtual void end_child_context(xmlns_id_t ns, xml_token_t name, xml_context_base* child);

    virtual void start_element(xmlns_id_t ns, xml_token_t name, const::std::vector<xml_token_attr_t>& attrs);

    virtual bool end_element(xmlns_id_t ns, xml_token_t name);

    virtual void characters(std::string_view str, bool transient);

private:
    void reset();
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
