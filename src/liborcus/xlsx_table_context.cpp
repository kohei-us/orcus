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

namespace {

class table_column_attr_parser
{
    string_pool* m_pool;

    long m_id;
    pstring m_name;
    pstring m_totals_row_label;
    spreadsheet::totals_row_function_t m_totals_row_func;

public:
    table_column_attr_parser(string_pool* pool) :
        m_pool(pool), m_id(-1), m_totals_row_func(spreadsheet::totals_row_function_t::none) {}

    void operator() (const xml_token_attr_t& attr)
    {
        if (attr.ns && attr.ns != NS_ooxml_xlsx)
            return;

        switch (attr.name)
        {
            case XML_id:
                m_id = to_long(attr.value);
                break;
            case XML_name:
                m_name = attr.value;
                if (attr.transient)
                    m_name = m_pool->intern(m_name).first;
                break;
            case XML_totalsRowLabel:
                m_totals_row_label = attr.value;
                if (attr.transient)
                    m_totals_row_label = m_pool->intern(m_totals_row_label).first;
                break;
            case XML_totalsRowFunction:
                m_totals_row_func = spreadsheet::to_totals_row_function_enum(attr.value);
                break;
            default:
                ;
        }
    }

    long get_id() const { return m_id; }
    pstring get_name() const { return m_name; }
    pstring get_totals_row_label() const { return m_totals_row_label; }
    spreadsheet::totals_row_function_t get_totals_row_function() const { return m_totals_row_func; }
};

class table_style_info_attr_parser
{
    spreadsheet::iface::import_table* mp_table;
    bool m_debug;

public:
    table_style_info_attr_parser(spreadsheet::iface::import_table* table, bool debug) :
        mp_table(table), m_debug(debug) {}

    void operator() (const xml_token_attr_t& attr)
    {
        if (attr.ns && attr.ns != NS_ooxml_xlsx)
            return;

        bool b = false;

        switch (attr.name)
        {
            case XML_name:
                mp_table->set_style_name(attr.value);
                if (m_debug)
                    std::cout << "  * table style info (name=" << attr.value << ")" << std::endl;
            break;
            case XML_showFirstColumn:
                b = to_bool(attr.value);
                mp_table->set_style_show_first_column(b);
                if (m_debug)
                    std::cout << "    * show first column: " << b << std::endl;
            break;
            case XML_showLastColumn:
                b = to_bool(attr.value);
                mp_table->set_style_show_last_column(b);
                if (m_debug)
                    std::cout << "    * show last column: " << b << std::endl;
            break;
            case XML_showRowStripes:
                b = to_bool(attr.value);
                mp_table->set_style_show_row_stripes(b);
                if (m_debug)
                    std::cout << "    * show row stripes: " << b << std::endl;
            break;
            case XML_showColumnStripes:
                b = to_bool(attr.value);
                mp_table->set_style_show_column_stripes(b);
                if (m_debug)
                    std::cout << "    * show column stripes: " << b << std::endl;
            break;
            default:
                ;
        }
    }
};

}

xlsx_table_context::xlsx_table_context(
    session_context& session_cxt, const tokens& tokens,
    spreadsheet::iface::import_table& table,
    spreadsheet::iface::import_reference_resolver& resolver) :
    xml_context_base(session_cxt, tokens), m_table(table), m_resolver(resolver),
    m_cxt_autofilter(session_cxt, tokens, resolver)
{
    register_child(&m_cxt_autofilter);

    init_ooxml_context(*this);
}

xlsx_table_context::~xlsx_table_context() {}

xml_context_base* xlsx_table_context::create_child_context(xmlns_id_t ns, xml_token_t name)
{
    if (ns == NS_ooxml_xlsx && name == XML_autoFilter)
    {
        m_cxt_autofilter.reset();
        return &m_cxt_autofilter;
    }
    return nullptr;
}

void xlsx_table_context::end_child_context(xmlns_id_t ns, xml_token_t name, xml_context_base* child)
{
    if (ns == NS_ooxml_xlsx && name == XML_autoFilter)
    {
        assert(child == &m_cxt_autofilter);

        spreadsheet::iface::import_auto_filter* af = m_table.get_auto_filter();
        if (!af)
            return;

        const xlsx_autofilter_context& cxt = static_cast<const xlsx_autofilter_context&>(*child);
        cxt.push_to_model(*af);
    }
}

void xlsx_table_context::start_element(xmlns_id_t ns, xml_token_t name, const xml_token_attrs_t& attrs)
{
    xml_token_pair_t parent = push_stack(ns, name);
    if (ns != NS_ooxml_xlsx)
        return;

    std::string_view str;

    switch (name)
    {
        case XML_table:
        {
            xml_element_expected(parent, XMLNS_UNKNOWN_ID, XML_UNKNOWN_TOKEN);
            start_element_table(attrs);
            break;
        }
        case XML_tableColumns:
        {
            xml_element_expected(parent, NS_ooxml_xlsx, XML_table);
            single_long_attr_getter func(NS_ooxml_xlsx, XML_count);
            long column_count = for_each(attrs.begin(), attrs.end(), func).get_value();
            if (get_config().debug)
                std::cout << "  * column count: " << column_count << std::endl;

            m_table.set_column_count(column_count);
        }
        break;
        case XML_tableColumn:
        {
            xml_element_expected(parent, NS_ooxml_xlsx, XML_tableColumns);
            table_column_attr_parser func(&get_session_context().spool);
            func = for_each(attrs.begin(), attrs.end(), func);
            if (get_config().debug)
            {
                std::cout << "  * table column (id=" << func.get_id() << "; name=" << func.get_name() << ")" << std::endl;
                std::cout << "    * totals row label: " << func.get_totals_row_label() << std::endl;
                std::cout << "    * totals func: " << static_cast<int>(func.get_totals_row_function()) << std::endl;
            }

            m_table.set_column_identifier(func.get_id());
            str = func.get_name();
            m_table.set_column_name(str);
            str = func.get_totals_row_label();
            m_table.set_column_totals_row_label(str);
            m_table.set_column_totals_row_function(func.get_totals_row_function());
        }
        break;
        case XML_tableStyleInfo:
        {
            xml_element_expected(parent, NS_ooxml_xlsx, XML_table);
            table_style_info_attr_parser func(&m_table, get_config().debug);
            for_each(attrs.begin(), attrs.end(), func);
        }
        break;
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

void xlsx_table_context::start_element_table(const xml_token_attrs_t& attrs)
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

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
