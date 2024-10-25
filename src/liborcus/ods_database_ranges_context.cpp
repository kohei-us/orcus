/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "ods_database_ranges_context.hpp"
#include "odf_token_constants.hpp"
#include "odf_namespace_types.hpp"

#include <orcus/spreadsheet/import_interface.hpp>
#include <orcus/spreadsheet/import_interface_auto_filter.hpp>

#include <sstream>

namespace ss = orcus::spreadsheet;

namespace orcus {

ods_database_ranges_context::ods_database_ranges_context(
    session_context& session_cxt, const tokens& tokens, ss::iface::import_factory* factory) :
    xml_context_base(session_cxt, tokens),
    mp_factory(factory)
{
    static const xml_element_validator::rule rules[] = {
        // parent element -> child element
        { XMLNS_UNKNOWN_ID, XML_UNKNOWN_TOKEN, NS_odf_table, XML_database_ranges }, // root element
        { NS_odf_table, XML_database_range, NS_odf_table, XML_filter },
        { NS_odf_table, XML_database_ranges, NS_odf_table, XML_database_range },
        { NS_odf_table, XML_filter, NS_odf_table, XML_filter_and },
        { NS_odf_table, XML_filter, NS_odf_table, XML_filter_or },
        { NS_odf_table, XML_filter_and, NS_odf_table, XML_filter_condition },
        { NS_odf_table, XML_filter_and, NS_odf_table, XML_filter_or },
        { NS_odf_table, XML_filter_or, NS_odf_table, XML_filter_and },
        { NS_odf_table, XML_filter_or, NS_odf_table, XML_filter_condition },
    };

    init_element_validator(rules, std::size(rules));
}

ods_database_ranges_context::~ods_database_ranges_context() = default;

void ods_database_ranges_context::start_element(
    xmlns_id_t ns, xml_token_t name, const xml_token_attrs_t& attrs)
{
    push_stack(ns, name);

    if (ns != NS_odf_table)
        warn_unexpected();

    switch (name)
    {
        case XML_database_ranges:
            break;
        case XML_database_range:
            start_database_range(attrs);
            break;
        case XML_filter:
            start_filter(attrs);
            break;
        case XML_filter_or:
            warn_unhandled();
            break;
        case XML_filter_and:
            warn_unhandled();
            break;
        case XML_filter_condition:
            warn_unhandled();
            break;
        default:
            warn_unexpected();
    }
}

bool ods_database_ranges_context::end_element(xmlns_id_t ns, xml_token_t name)
{
    if (ns == NS_odf_table)
    {
        switch (name)
        {
            case XML_database_range:
                end_database_range();
                break;
            case XML_filter:
                end_filter();
                break;
        }
    }
    return pop_stack(ns, name);
}

void ods_database_ranges_context::reset()
{
    mp_table = nullptr;
    mp_filter = nullptr;

    m_target_range.reset();
}

void ods_database_ranges_context::start_database_range(const xml_token_attrs_t& attrs)
{
    if (!mp_factory)
        return;

    auto resolver = mp_factory->get_reference_resolver(ss::formula_ref_context_t::table_range);
    if (!resolver)
        return;

    std::string_view name;
    std::optional<ss::src_range_t> target_range;

    for (const auto& attr : attrs)
    {
        if (attr.ns != NS_odf_table)
            continue;

        switch (attr.name)
        {
            case XML_name:
                name = attr.value;
                break;
            case XML_target_range_address:
                target_range = resolver->resolve_range(attr.value);
                break;
        }
    }

    if (!target_range)
        return;

    if (name.empty())
        return;

    auto* sh = mp_factory->get_sheet(target_range->first.sheet); // Just use the first sheet for now
    if (!sh)
        return;

    if (target_range->last.sheet > target_range->first.sheet)
    {
        std::ostringstream os;
        os << "database range named '" << name << "' spans across multiple sheets, but only the first sheet is used";
        warn(os.str());
    }

    mp_table = sh->start_table();
    if (!mp_table)
        return;

    m_target_range = to_rc_range(*target_range);

    mp_table->set_name(name);
    mp_table->set_range(*m_target_range);
}

void ods_database_ranges_context::end_database_range()
{
    if (!mp_table)
        return;

    mp_table->commit();
    mp_table = nullptr;
}

void ods_database_ranges_context::start_filter(const xml_token_attrs_t& /*attrs*/)
{
    if (!mp_table || !m_target_range)
        return;

    mp_filter = mp_table->start_auto_filter(*m_target_range);
}

void ods_database_ranges_context::end_filter()
{
    mp_filter->commit();
    mp_filter = nullptr;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

