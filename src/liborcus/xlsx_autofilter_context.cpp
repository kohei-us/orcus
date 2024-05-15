/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "xlsx_autofilter_context.hpp"
#include "xml_context_global.hpp"
#include "ooxml_namespace_types.hpp"
#include "ooxml_token_constants.hpp"

#include "orcus/spreadsheet/import_interface.hpp"

namespace orcus {

xlsx_autofilter_context::xlsx_autofilter_context(
    session_context& session_cxt, const tokens& tokens,
    spreadsheet::iface::import_reference_resolver& resolver) :
    xml_context_base(session_cxt, tokens),
    m_resolver(resolver),
    m_cur_col(-1) {}

xlsx_autofilter_context::~xlsx_autofilter_context() = default;

void xlsx_autofilter_context::start_element(xmlns_id_t ns, xml_token_t name, const xml_token_attrs_t& attrs)
{
    xml_token_pair_t parent = push_stack(ns, name);

    if (ns != NS_ooxml_xlsx)
        return;

    switch (name)
    {
        case XML_autoFilter:
        {
            xml_element_expected(parent, XMLNS_UNKNOWN_ID, XML_UNKNOWN_TOKEN);
            m_ref_range = get_single_attr(attrs, NS_ooxml_xlsx, XML_ref, &m_pool);
            break;
        }
        case XML_filterColumn:
        {
            xml_element_expected(parent, NS_ooxml_xlsx, XML_autoFilter);
            m_cur_col = -1;
            if (auto v = get_single_long_attr(attrs, NS_ooxml_xlsx, XML_colId); v)
                m_cur_col = *v;
            else
                throw xml_structure_error("failed to parse a column id (colId) from filterColumn");

            break;
        }
        case XML_filters:
            xml_element_expected(parent, NS_ooxml_xlsx, XML_filterColumn);
            break;
        case XML_filter:
        {
            xml_element_expected(parent, NS_ooxml_xlsx, XML_filters);
            if (std::string_view val = get_single_attr(attrs, NS_ooxml_xlsx, XML_val, &m_pool); !val.empty())
                m_cur_match_values.push_back(val);
            break;
        }
        default:
            warn_unhandled();
    }
}

bool xlsx_autofilter_context::end_element(xmlns_id_t ns, xml_token_t name)
{
    if (ns == NS_ooxml_xlsx)
    {
        switch (name)
        {
            case XML_filterColumn:
            {
                if (m_cur_col >= 0)
                {
                    m_column_filters.insert(
                        column_filters_type::value_type(m_cur_col, m_cur_match_values));
                }
                m_cur_col = -1;
                m_cur_match_values.clear();
            }
            break;
            default:
                ;
        }
    }
    return pop_stack(ns, name);
}

void xlsx_autofilter_context::characters(std::string_view /*str*/, bool /*transient*/)
{
}

void xlsx_autofilter_context::push_to_model(spreadsheet::iface::import_auto_filter& af) const
{
    spreadsheet::src_range_t range = m_resolver.resolve_range(m_ref_range);
    af.set_range(to_rc_range(range));

    for (const auto& v : m_column_filters)
    {
        spreadsheet::col_t col = v.first;
        const match_values_type& match_values = v.second;

        af.set_column(col);
        for (std::string_view value : match_values)
            af.append_column_match_value(value);
        af.commit_column();
    }
    af.commit();
}

void xlsx_autofilter_context::reset()
{
    m_pool.clear();
    m_ref_range = std::string_view{};
    m_cur_col = -1;
    m_cur_match_values.clear();
    m_column_filters.clear();
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
