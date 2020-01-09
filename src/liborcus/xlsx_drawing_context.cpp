/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "xlsx_drawing_context.hpp"
#include "ooxml_namespace_types.hpp"
#include "ooxml_token_constants.hpp"
#include "ooxml_global.hpp"

#include "orcus/measurement.hpp"

#include <iostream>

using namespace std;

namespace orcus {

xlsx_drawing_context::xlsx_drawing_context(session_context& cxt, const tokens& tkns) :
    xml_context_base(cxt, tkns),
    m_col(-1), m_row(-1), m_col_offset(-1), m_row_offset(-1)
{
    init_ooxml_context(*this);
}

xlsx_drawing_context::~xlsx_drawing_context() {}

bool xlsx_drawing_context::can_handle_element(xmlns_id_t /*ns*/, xml_token_t /*name*/) const
{
    return true;
}

xml_context_base* xlsx_drawing_context::create_child_context(xmlns_id_t /*ns*/, xml_token_t /*name*/)
{
    return nullptr;
}

void xlsx_drawing_context::end_child_context(xmlns_id_t /*ns*/, xml_token_t /*name*/, xml_context_base* /*child*/)
{
}

void xlsx_drawing_context::start_element(xmlns_id_t ns, xml_token_t name, const::std::vector<xml_token_attr_t>& attrs)
{
    xml_token_pair_t parent = push_stack(ns, name);

    if (ns == NS_ooxml_xdr)
    {
        switch (name)
        {
            case XML_oneCellAnchor:
            case XML_twoCellAnchor:
            {
                xml_element_expected(parent, NS_ooxml_xdr, XML_wsDr);
                reset();
                break;
            }
            case XML_from:
            case XML_sp:
            case XML_clientData:
            {
                const xml_elem_set_t expected = {
                    { NS_ooxml_xdr, XML_grpSp },
                    { NS_ooxml_xdr, XML_oneCellAnchor },
                    { NS_ooxml_xdr, XML_twoCellAnchor },
                };
                xml_element_expected(parent, expected);
                break;
            }
            case XML_to:
            {
                xml_element_expected(parent, NS_ooxml_xdr, XML_twoCellAnchor);
                break;
            }
            case XML_col:
            case XML_colOff:
            case XML_row:
            case XML_rowOff:
            {
                xml_elem_stack_t expected;
                expected.emplace_back(NS_ooxml_xdr, XML_from);
                expected.emplace_back(NS_ooxml_xdr, XML_to);
                xml_element_expected(parent, expected);
                break;
            }
            case XML_nvSpPr:
            case XML_style:
            case XML_txBody:
            {
                const xml_elem_stack_t expected = {
                    { NS_ooxml_xdr, XML_cxnSp },
                    { NS_ooxml_xdr, XML_sp },
                };
                xml_element_expected(parent, expected);
                break;
            }
            case XML_spPr:
            {
                const xml_elem_stack_t expected = {
                    { NS_ooxml_xdr, XML_cxnSp },
                    { NS_ooxml_xdr, XML_sp },
                    { NS_ooxml_xdr, XML_pic },
                };
                xml_element_expected(parent, expected);
                break;
            }
            default:
                warn_unhandled();
        }
    }
    else if (ns == NS_ooxml_a)
    {
        warn_unhandled();
    }
    else
        warn_unhandled();
}

bool xlsx_drawing_context::end_element(xmlns_id_t ns, xml_token_t name)
{
    if (ns == NS_ooxml_xdr)
    {
        switch (name)
        {
            case XML_twoCellAnchor:
            case XML_oneCellAnchor:
                cout << "col: " << m_col << "; row: " << m_row << "; col offset: " << m_col_offset << "; row offset: " << m_row_offset << endl;
                break;
            default:
                ;
        }
    }
    else if (ns == NS_ooxml_a)
    {

    }
    return pop_stack(ns, name);
}

void xlsx_drawing_context::characters(const pstring& str, bool /*transient*/)
{
    const xml_token_pair_t& elem = get_current_element();
    if (elem.first == NS_ooxml_xdr)
    {
        switch (elem.second)
        {
            case XML_col:
                m_col = to_long(str);
                break;
            case XML_row:
                m_row = to_long(str);
                break;
            case XML_colOff:
                m_col_offset = to_long(str);
                break;
            case XML_rowOff:
                m_row_offset = to_long(str);
                break;
            default:
                ;
        }
    }
}

void xlsx_drawing_context::reset()
{
    m_col = -1;
    m_row = -1;
    m_col_offset = -1;
    m_row_offset = -1;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
