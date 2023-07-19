/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "gnumeric_context.hpp"
#include "gnumeric_token_constants.hpp"
#include "gnumeric_namespace_types.hpp"
#include "gnumeric_sheet_context.hpp"
#include "orcus/spreadsheet/import_interface.hpp"

#include <fstream>
#include <algorithm>

namespace ss = orcus::spreadsheet;

namespace orcus {

gnumeric_content_xml_context::gnumeric_content_xml_context(
    session_context& session_cxt, const tokens& tokens, spreadsheet::iface::import_factory* factory) :
    xml_context_base(session_cxt, tokens),
    mp_factory(factory),
    m_sheet_count(0),
    m_cxt_names(session_cxt, tokens, factory),
    m_cxt_sheet(session_cxt, tokens, factory)
{
    static const xml_element_validator::rule rules[] = {
        // parent element -> child element
        { XMLNS_UNKNOWN_ID, XML_UNKNOWN_TOKEN, NS_gnumeric_gnm, XML_Workbook }, // root element
        { NS_gnumeric_gnm, XML_SheetNameIndex, NS_gnumeric_gnm, XML_SheetName },
        { NS_gnumeric_gnm, XML_Sheets, NS_gnumeric_gnm, XML_Sheet },
        { NS_gnumeric_gnm, XML_Workbook, NS_gnumeric_gnm, XML_Names },
        { NS_gnumeric_gnm, XML_Workbook, NS_gnumeric_gnm, XML_SheetNameIndex },
        { NS_gnumeric_gnm, XML_Workbook, NS_gnumeric_gnm, XML_Sheets },
    };

    init_element_validator(rules, std::size(rules));

    register_child(&m_cxt_names);
    register_child(&m_cxt_sheet);
}

gnumeric_content_xml_context::~gnumeric_content_xml_context() = default;

xml_context_base* gnumeric_content_xml_context::create_child_context(xmlns_id_t ns, xml_token_t name)
{
    if (ns == NS_gnumeric_gnm)
    {
        switch (name)
        {
            case XML_Sheet:
            {
                m_cxt_sheet.reset();
                return &m_cxt_sheet;
            }
            case XML_Names:
            {
                m_cxt_names.reset();
                return &m_cxt_names;
            }
        }
    }

    return nullptr;
}

void gnumeric_content_xml_context::end_child_context(
    xmlns_id_t ns, xml_token_t name, xml_context_base* child)
{
    if (ns == NS_gnumeric_gnm)
    {
        switch (name)
        {
            case XML_Names:
            {
                assert(child == &m_cxt_names);
                end_names();
                break;
            }
        }
    }
}

void gnumeric_content_xml_context::start_element(xmlns_id_t ns, xml_token_t name, const xml_token_attrs_t& /*attrs*/)
{
    push_stack(ns, name);

    if (ns == NS_gnumeric_gnm)
    {
        switch (name)
        {
            case XML_SheetName:
                break;
            default:
                warn_unhandled();
        }
    }
    else
        warn_unhandled();
}

bool gnumeric_content_xml_context::end_element(xmlns_id_t ns, xml_token_t name)
{
    return pop_stack(ns, name);
}

void gnumeric_content_xml_context::characters(std::string_view str, bool /*transient*/)
{
    if (str.empty())
        return;

    auto current = get_current_element();

    if (current.first == NS_gnumeric_gnm)
    {
        switch (current.second)
        {
            case XML_SheetName:
            {
                auto* p = mp_factory->append_sheet(m_sheet_count++, str);
                if (!p)
                {
                    std::ostringstream os;
                    os << "failed to append a new sheet named '" << str << "'";
                    warn(os.str());
                }

                break;
            }
        }
    }
}

void gnumeric_content_xml_context::end_names()
{
    ss::iface::import_named_expression* named_exp = mp_factory->get_named_expression();
    if (!named_exp)
        return;

    for (const auto& name : m_cxt_names.get_names())
    {
        try
        {
            named_exp->set_base_position(name.position);
            named_exp->set_named_expression(name.name, name.value);
            named_exp->commit();
        }
        catch (const std::exception& e)
        {
            std::ostringstream os;
            os << "failed to commit a named expression named '" << name.name
                << "': (reason='" << e.what() << "'; value='" << name.value << "')";
            warn(os.str());
        }
    }
}

} // namespace orcus

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
