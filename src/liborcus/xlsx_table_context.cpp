/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "xlsx_table_context.hpp"
#include "xlsx_autofilter_context.hpp"
#include "ooxml_namespace_types.hpp"
#include "ooxml_token_constants.hpp"
#include "ooxml_global.hpp"
#include "session_context.hpp"
#include "xml_context_global.hpp"

#include "orcus/measurement.hpp"
#include "orcus/spreadsheet/import_interface.hpp"

#include <iostream>
#include <optional>

namespace ss = orcus::spreadsheet;

namespace orcus {

xlsx_table_context::xlsx_table_context(
    session_context& session_cxt, const tokens& tokens,
    spreadsheet::iface::import_table& table,
    spreadsheet::iface::import_reference_resolver& resolver) :
    xml_context_base(session_cxt, tokens), m_table(table), m_resolver(resolver),
    m_cxt_autofilter(session_cxt, tokens, resolver)
{
    static const xml_element_validator::rule rules[] = {
        // parent element -> child element
        { XMLNS_UNKNOWN_ID, XML_UNKNOWN_TOKEN, NS_ooxml_xlsx, XML_table }, // root element
        { NS_ooxml_xlsx, XML_table, NS_ooxml_xlsx, XML_tableColumns },
        { NS_ooxml_xlsx, XML_table, NS_ooxml_xlsx, XML_tableStyleInfo },
        { NS_ooxml_xlsx, XML_tableColumns, NS_ooxml_xlsx, XML_tableColumn },
    };

    init_element_validator(rules, std::size(rules));
    register_child(&m_cxt_autofilter);
    init_ooxml_context(*this);
}

xlsx_table_context::~xlsx_table_context() = default;

xml_context_base* xlsx_table_context::create_child_context(xmlns_id_t ns, xml_token_t name)
{
    if (ns == NS_ooxml_xlsx && name == XML_autoFilter)
    {
        xlsx_autofilter_context::iface_factory_type func = [](const ss::range_t& range)
        {
            return nullptr;
        };

        m_cxt_autofilter.reset(std::move(func));
        return &m_cxt_autofilter;
    }
    return nullptr;
}

void xlsx_table_context::start_element(xmlns_id_t ns, xml_token_t name, const xml_token_attrs_t& attrs)
{
    push_stack(ns, name);

    if (ns != NS_ooxml_xlsx)
        return;

    switch (name)
    {
        case XML_table:
        {
            start_table(attrs);
            break;
        }
        case XML_tableColumns:
        {
            start_table_columns(attrs);
            break;
        }
        case XML_tableColumn:
        {
            start_table_column(attrs);
            break;
        }
        case XML_tableStyleInfo:
        {
            start_table_style_info(attrs);
            break;
        }
        default:
            warn_unhandled();
    }

}

bool xlsx_table_context::end_element(xmlns_id_t ns, xml_token_t name)
{
    if (ns == NS_ooxml_xlsx)
    {
        switch (name)
        {
            case XML_table:
                m_table.commit();
            break;
            case XML_tableColumn:
                m_table.commit_column();
            break;
            default:
                ;
        }
    }

    return pop_stack(ns, name);
}

void xlsx_table_context::start_table(const xml_token_attrs_t& attrs)
{
    long id = -1;
    long totals_row_count = -1;

    std::optional<std::string_view> name;
    std::optional<std::string_view> display_name;
    std::optional<std::string_view> ref;

    for (const xml_token_attr_t& attr : attrs)
    {
        if (attr.ns)
            continue;

        switch (attr.name)
        {
            case XML_id:
                id = to_long(attr.value);
                break;
            case XML_totalsRowCount:
                totals_row_count = to_long(attr.value);
                break;
            case XML_name:
                name = attr.value;
                break;
            case XML_displayName:
                display_name = attr.value;
                break;
            case XML_ref:
                ref = attr.value;
                break;
        }
    }

    if (get_config().debug)
    {
        auto str_or_not = [](const auto& v) -> std::string_view
        {
            return v ? *v : "-";
        };

        std::cout << "* table (range=" << str_or_not(ref)
             << "; id=" << id
             << "; name=" << str_or_not(name)
             << "; display name=" << str_or_not(display_name) << ")" << std::endl;

        std::cout << "  * totals row count: " << totals_row_count << std::endl;
    }

    if (id >= 0)
        m_table.set_identifier(id);

    if (ref)
    {
        ss::range_t range = to_rc_range(m_resolver.resolve_range(*ref));
        m_table.set_range(range);
    }

    if (name)
        m_table.set_name(*name);

    if (display_name)
        m_table.set_display_name(*display_name);

    if (totals_row_count >= 0)
        m_table.set_totals_row_count(totals_row_count);
}

void xlsx_table_context::start_table_columns(const xml_token_attrs_t& attrs)
{
    if (auto v = get_single_long_attr(attrs, NS_ooxml_xlsx, XML_count); v)
    {
        if (get_config().debug)
            std::cout << "  * column count: " << *v << std::endl;

        m_table.set_column_count(*v);
    }
    else
        throw xml_structure_error("failed to get a column count from tableColumns");
}

void xlsx_table_context::start_table_column(const xml_token_attrs_t& attrs)
{
    long col_id = -1;
    std::string_view name;
    std::string_view totals_row_label;
    auto totals_row_func = spreadsheet::totals_row_function_t::none;

    for (const auto& attr : attrs)
    {
        if (attr.ns)
            continue;

        switch (attr.name)
        {
            case XML_id:
                col_id = to_long(attr.value);
                break;
            case XML_name:
                name = attr.value;
                break;
            case XML_totalsRowLabel:
                totals_row_label = attr.value;
                break;
            case XML_totalsRowFunction:
                totals_row_func = spreadsheet::to_totals_row_function_enum(attr.value);
                break;
        }
    }

    if (get_config().debug)
    {
        std::cout << "  * table column (id=" << col_id << "; name=" << name << ")" << std::endl;
        std::cout << "    * totals row label: " << totals_row_label << std::endl;
        std::cout << "    * totals func: " << int(totals_row_func) << std::endl;
    }

    m_table.set_column_identifier(col_id);
    m_table.set_column_name(name);
    m_table.set_column_totals_row_label(totals_row_label);
    m_table.set_column_totals_row_function(totals_row_func);
}

void xlsx_table_context::start_table_style_info(const xml_token_attrs_t& attrs)
{
    bool debug = get_config().debug;

    for (const auto& attr : attrs)
    {
        if (attr.ns)
            continue;

        switch (attr.name)
        {
            case XML_name:
            {
                m_table.set_style_name(attr.value);
                if (debug)
                    std::cout << "  * table style info (name=" << attr.value << ")" << std::endl;
                break;
            }
            case XML_showFirstColumn:
            {
                bool b = to_bool(attr.value);
                m_table.set_style_show_first_column(b);
                if (debug)
                    std::cout << "    * show first column: " << b << std::endl;
                break;
            }
            case XML_showLastColumn:
            {
                bool b = to_bool(attr.value);
                m_table.set_style_show_last_column(b);
                if (debug)
                    std::cout << "    * show last column: " << b << std::endl;
                break;
            }
            case XML_showRowStripes:
            {
                bool b = to_bool(attr.value);
                m_table.set_style_show_row_stripes(b);
                if (debug)
                    std::cout << "    * show row stripes: " << b << std::endl;
                break;
            }
            case XML_showColumnStripes:
            {
                bool b = to_bool(attr.value);
                m_table.set_style_show_column_stripes(b);
                if (debug)
                    std::cout << "    * show column stripes: " << b << std::endl;
                break;
            }
        }
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
