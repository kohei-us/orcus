/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "xlsx_workbook_context.hpp"
#include "ooxml_global.hpp"
#include "ooxml_schemas.hpp"
#include "ooxml_token_constants.hpp"
#include "ooxml_namespace_types.hpp"
#include "session_context.hpp"
#include "xlsx_session_data.hpp"

#include "orcus/global.hpp"
#include "orcus/measurement.hpp"
#include "orcus/spreadsheet/import_interface.hpp"

using namespace std;

namespace orcus {

xlsx_workbook_context::xlsx_workbook_context(
    session_context& session_cxt, const tokens& tokens,
    spreadsheet::iface::import_named_expression* named_exp) :
    xml_context_base(session_cxt, tokens),
    mp_named_exp(named_exp) {}

xlsx_workbook_context::~xlsx_workbook_context() {}

bool xlsx_workbook_context::can_handle_element(xmlns_id_t /*ns*/, xml_token_t /*name*/) const
{
    return true;
}

xml_context_base* xlsx_workbook_context::create_child_context(xmlns_id_t /*ns*/, xml_token_t /*name*/)
{
    return nullptr;
}

void xlsx_workbook_context::end_child_context(xmlns_id_t /*ns*/, xml_token_t /*name*/, xml_context_base* /*child*/)
{
}

void xlsx_workbook_context::start_element(xmlns_id_t ns, xml_token_t name, const xml_attrs_t& attrs)
{
    xml_token_pair_t parent = push_stack(ns, name);
    session_context& cxt = get_session_context();
    string_pool& sp = cxt.m_string_pool;
    xlsx_session_data& sdata = static_cast<xlsx_session_data&>(*cxt.mp_data);

    switch (name)
    {
        case XML_workbook:
        {
            xml_element_expected(parent, XMLNS_UNKNOWN_ID, XML_UNKNOWN_TOKEN);
            if (get_config().debug)
                print_attrs(get_tokens(), attrs);

            break;
        }
        case XML_sheets:
            xml_element_expected(parent, NS_ooxml_xlsx, XML_workbook);
            break;
        case XML_sheet:
        {
            xml_element_expected(parent, NS_ooxml_xlsx, XML_sheets);

            pstring rid;
            xlsx_rel_sheet_info sheet;

            std::for_each(attrs.begin(), attrs.end(),
                [&](const xml_token_attr_t& attr)
                {
                    if (attr.ns == NS_ooxml_xlsx)
                    {
                        switch (attr.name)
                        {
                            case XML_name:
                                sheet.name = sp.intern(attr.value).first;
                                break;
                            case XML_sheetId:
                            {
                                const pstring& val = attr.value;
                                if (!val.empty())
                                    sheet.id = to_long(val);
                                break;
                            }
                            default:
                                ;
                        }
                    }
                    else if (attr.ns == NS_ooxml_r && attr.name == XML_id)
                    {
                        rid = sp.intern(attr.value).first;
                    }
                }
            );

            if (sheet.id > 0)
                // Excel's sheet ID is 1-based. Convert it to 0-based.
                sdata.set_sheet_name_map(sheet.name, sheet.id-1);

            m_workbook_info.data.insert(
                opc_rel_extras_t::map_type::value_type(
                    rid, orcus::make_unique<xlsx_rel_sheet_info>(sheet)));

            break;
        }
        case XML_definedNames:
            xml_element_expected(parent, NS_ooxml_xlsx, XML_workbook);
            break;
        case XML_definedName:
        {
            xml_element_expected(parent, NS_ooxml_xlsx, XML_definedNames);

            std::for_each(attrs.begin(), attrs.end(),
                [&](const xml_token_attr_t& attr)
                {
                    if (attr.ns != NS_ooxml_xlsx)
                        return;

                    if (attr.name == XML_name)
                    {
                        m_defined_name = attr.value;
                        if (attr.transient)
                        {
                            m_defined_name =
                                cxt.m_string_pool.intern(m_defined_name).first;
                        }
                    }
                }
            );

            break;
        }
        case XML_pivotCaches:
            xml_element_expected(parent, NS_ooxml_xlsx, XML_workbook);
            break;
        case XML_pivotCache:
        {
            xml_element_expected(parent, NS_ooxml_xlsx, XML_pivotCaches);

            pstring rid;
            long cache_id = -1;
            for_each(attrs.begin(), attrs.end(),
                [&](const xml_token_attr_t& attr)
                {
                    if (attr.ns == NS_ooxml_xlsx && attr.name == XML_cacheId)
                    {
                        cache_id = to_long(attr.value);
                    }
                    else if (attr.ns == NS_ooxml_r && attr.name == XML_id)
                    {
                        rid = attr.value;
                    }
                }
            );

            m_workbook_info.data.insert(
                opc_rel_extras_t::map_type::value_type(
                    rid, orcus::make_unique<xlsx_rel_pivot_cache_info>(cache_id)));

            break;
        }
        default:
            warn_unhandled();
    }
}

bool xlsx_workbook_context::end_element(xmlns_id_t ns, xml_token_t name)
{
    if (ns == NS_ooxml_xlsx)
    {
        if (name == XML_definedName)
        {
            if (mp_named_exp)
            {
                mp_named_exp->define_name(
                    m_defined_name.data(), m_defined_name.size(),
                    m_defined_name_exp.data(), m_defined_name_exp.size());
            }
            m_defined_name.clear();
            m_defined_name_exp.clear();
        }
    }
    return pop_stack(ns, name);
}

void xlsx_workbook_context::characters(const pstring& str, bool transient)
{
    const xml_token_pair_t& cur = get_current_element();
    string_pool& sp = get_session_context().m_string_pool;

    if (cur.first == NS_ooxml_xlsx)
    {
        if (cur.second == XML_definedName)
            m_defined_name_exp = transient ? sp.intern(str).first : str;
    }
}

void xlsx_workbook_context::pop_workbook_info(opc_rel_extras_t& workbook_data)
{
    m_workbook_info.swap(workbook_data);
}

}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
