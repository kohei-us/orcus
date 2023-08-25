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

#include "orcus/measurement.hpp"
#include "orcus/spreadsheet/import_interface.hpp"

namespace orcus {

xlsx_workbook_context::xlsx_workbook_context(
    session_context& session_cxt, const tokens& tokens,
    spreadsheet::iface::import_factory& factory) :
    xml_context_base(session_cxt, tokens),
    m_defined_name_scope(-1),
    m_sheet_count(0),
    m_factory(factory),
    mp_named_exp(factory.get_named_expression())
{
    init_ooxml_context(*this);
}

xlsx_workbook_context::~xlsx_workbook_context() = default;

xml_context_base* xlsx_workbook_context::create_child_context(xmlns_id_t /*ns*/, xml_token_t /*name*/)
{
    return nullptr;
}

void xlsx_workbook_context::end_child_context(xmlns_id_t /*ns*/, xml_token_t /*name*/, xml_context_base* /*child*/)
{
}

void xlsx_workbook_context::start_element(xmlns_id_t ns, xml_token_t name, const xml_token_attrs_t& attrs)
{
    xml_token_pair_t parent = push_stack(ns, name);
    session_context& cxt = get_session_context();
    string_pool& sp = cxt.spool;

    if (ns == NS_ooxml_xlsx)
    {
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

                std::string_view rid;
                xlsx_rel_sheet_info sheet;

                std::for_each(attrs.begin(), attrs.end(),
                    [&](const xml_token_attr_t& attr)
                    {
                        if (!attr.ns || attr.ns == NS_ooxml_xlsx)
                        {
                            switch (attr.name)
                            {
                                case XML_name:
                                    sheet.name = sp.intern(attr.value).first;
                                    break;
                                case XML_sheetId:
                                {
                                    if (!attr.value.empty())
                                        sheet.id = to_long(attr.value);
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

                if (sheet.name.empty())
                    throw xml_structure_error("workbook.xml: sheet element must have a valid name element.");

                // Insert the sheet here so that we have all the sheets available
                // prior to parsing global named expressions.
                m_factory.append_sheet(m_sheet_count++, sheet.name);

                m_workbook_info.data.insert(
                    opc_rel_extras_t::map_type::value_type(
                        rid, std::make_unique<xlsx_rel_sheet_info>(sheet)));

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
                        if (attr.ns && attr.ns != NS_ooxml_xlsx)
                            return;

                        switch (attr.name)
                        {
                            case XML_name:
                            {
                                m_defined_name = attr.value;
                                if (attr.transient)
                                {
                                    m_defined_name =
                                        cxt.spool.intern(m_defined_name).first;
                                }
                                break;
                            }
                            case XML_localSheetId:
                                m_defined_name_scope = to_long(attr.value);
                                break;
                            default:
                                ;
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

                std::string_view rid;
                long cache_id = -1;
                std::for_each(attrs.begin(), attrs.end(),
                    [&](const xml_token_attr_t& attr)
                    {
                        if (!attr.ns || attr.ns == NS_ooxml_xlsx)
                        {
                            switch (attr.name)
                            {
                                case XML_cacheId:
                                    cache_id = to_long(attr.value);
                                    break;
                                default:
                                    ;
                            }
                        }
                        else if (attr.ns == NS_ooxml_r)
                        {
                            switch (attr.name)
                            {
                                case XML_id:
                                    rid = attr.value;
                                    break;
                                default:
                                    ;
                            }
                        }
                    }
                );

                m_workbook_info.data.insert(
                    opc_rel_extras_t::map_type::value_type(
                        rid, std::make_unique<xlsx_rel_pivot_cache_info>(cache_id)));

                break;
            }
            default:
                warn_unhandled();
        }
    }
}

bool xlsx_workbook_context::end_element(xmlns_id_t ns, xml_token_t name)
{
    if (ns == NS_ooxml_xlsx)
    {
        if (name == XML_definedName)
        {
            push_defined_name();

            m_defined_name = std::string_view{};
            m_defined_name_exp = std::string_view{};
            m_defined_name_scope = -1;
        }
    }
    return pop_stack(ns, name);
}

void xlsx_workbook_context::characters(std::string_view str, bool transient)
{
    std::string_view sv(str);
    xml_token_pair_t cur = get_current_element();
    string_pool& sp = get_session_context().spool;

    if (cur.first == NS_ooxml_xlsx)
    {
        if (cur.second == XML_definedName)
            m_defined_name_exp = transient ? sp.intern(sv).first : sv;
    }
}

void xlsx_workbook_context::pop_workbook_info(opc_rel_extras_t& workbook_data)
{
    m_workbook_info.swap(workbook_data);
}

void xlsx_workbook_context::push_defined_name()
{
    spreadsheet::iface::import_named_expression* named_exp = mp_named_exp;

    if (m_defined_name_scope >= 0)
    {
        // sheet local scope.
        spreadsheet::iface::import_sheet* sheet = m_factory.get_sheet(m_defined_name_scope);
        if (!sheet)
            return;

        named_exp = sheet->get_named_expression();
    }

    if (named_exp)
    {
        named_exp->set_named_expression(m_defined_name, m_defined_name_exp);
        named_exp->commit();
    }
}

}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
