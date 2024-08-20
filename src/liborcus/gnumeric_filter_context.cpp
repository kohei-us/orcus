/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "gnumeric_filter_context.hpp"
#include "gnumeric_token_constants.hpp"
#include "gnumeric_namespace_types.hpp"

#include <orcus/spreadsheet/import_interface.hpp>
#include <orcus/spreadsheet/import_interface_auto_filter.hpp>
#include <orcus/measurement.hpp>

#include <iostream>
#include <optional>

namespace ss = orcus::spreadsheet;

namespace orcus {

namespace {

enum gnumeric_filter_field_op_t
{
    filter_equal,
    filter_greaterThan,
    filter_lessThan,
    filter_greaterThanEqual,
    filter_lessThanEqual,
    filter_notEqual,
    filter_op_invalid
};

enum gnumeric_filter_field_type_t
{
    filter_expr,
    filter_blanks,
    filter_nonblanks,
    filter_type_invalid
};

} // anonymous namespace

gnumeric_filter_context::gnumeric_filter_context(
    session_context& session_cxt, const tokens& tokens,
    ss::iface::import_factory* factory) :
    xml_context_base(session_cxt, tokens),
    mp_factory(factory)
{
    static const xml_element_validator::rule rules[] = {
        // parent element -> child element
        { XMLNS_UNKNOWN_ID, XML_UNKNOWN_TOKEN, NS_gnumeric_gnm, XML_Filter }, // root element
        { NS_gnumeric_gnm, XML_Filter, NS_gnumeric_gnm, XML_Field },
    };

    init_element_validator(rules, std::size(rules));
}

gnumeric_filter_context::~gnumeric_filter_context() = default;

void gnumeric_filter_context::start_element(
    xmlns_id_t ns, xml_token_t name, const std::vector<xml_token_attr_t>& attrs)
{
    push_stack(ns, name);

    if (ns == NS_gnumeric_gnm)
    {
        switch (name)
        {
            case XML_Filter:
            {
                start_filter(attrs);
                break;
            }
            case XML_Field:
            {
                start_field(attrs);
                break;
            }
            default:
                warn_unhandled();
        }
    }
    else
        warn_unhandled();
}

bool gnumeric_filter_context::end_element(xmlns_id_t ns, xml_token_t name)
{
    if (ns == NS_gnumeric_gnm)
    {
        switch (name)
        {
            case XML_Filter:
                end_filter();
                break;
            case XML_Field:
                end_field();
                break;
        }
    }

    return pop_stack(ns, name);
}

void gnumeric_filter_context::reset(spreadsheet::iface::import_sheet* sheet)
{
    mp_sheet = sheet;
    mp_auto_filter = nullptr;
}

void gnumeric_filter_context::start_filter(const xml_token_attrs_t& attrs)
{
    if (!mp_sheet)
        return;

    ss::iface::import_reference_resolver* resolver =
        mp_factory->get_reference_resolver(ss::formula_ref_context_t::global);

    if (!resolver)
        return;

    mp_auto_filter = mp_sheet->get_auto_filter();
    if (!mp_auto_filter)
        return;

    std::optional<spreadsheet::range_t> area;

    for (const xml_token_attr_t& attr : attrs)
    {
        switch (attr.name)
        {
            case XML_Area:
                area = to_rc_range(resolver->resolve_range(attr.value));
                break;
            default:
                ;
        }
    }

    if (!area)
    {
        mp_auto_filter = nullptr;
        return;
    }

    mp_auto_filter->set_range(*area);
}

void gnumeric_filter_context::start_field(const xml_token_attrs_t& attrs)
{
    if (!mp_auto_filter)
        return;

    gnumeric_filter_field_type_t filter_field_type = filter_type_invalid;
    gnumeric_filter_field_op_t filter_op = filter_op_invalid;

    ss::col_t col = -1;
    std::string_view filter_value_type;
    std::string_view filter_value;

    for (const xml_token_attr_t& attr : attrs)
    {
        switch (attr.name)
        {
            case XML_Index:
            {
                col = to_long(attr.value.data());
                break;
            }
            case XML_Type:
            {
                if (attr.value == "expr")
                    filter_field_type = filter_expr;
                else if (attr.value == "blanks")
                    filter_field_type = filter_blanks;
                else if (attr.value == "nonblanks")
                    filter_field_type = filter_nonblanks;
                break;
            }
            case XML_Op0:
            {
                if (attr.value == "eq")
                    filter_op = filter_equal;
                else if (attr.value == "gt")
                    filter_op = filter_greaterThan;
                else if (attr.value == "lt")
                    filter_op = filter_lessThan;
                else if (attr.value == "gte")
                    filter_op = filter_greaterThanEqual;
                else if (attr.value == "lte")
                    filter_op = filter_lessThanEqual;
                else if (attr.value == "ne")
                    filter_op = filter_notEqual;
                break;
            }
            case XML_Value0:
            {
                filter_value_type = attr.value;
                break;
            }
            case XML_ValueType0:
            {
                filter_value = attr.value;
                break;
            }
        }
    }

    if (col < 0)
        return;

    mp_auto_filter->set_column(col);

    switch (filter_field_type)
    {
        case filter_expr:
        {
            // only equal supported in API yet
            if (filter_op != filter_equal)
                return;

            // import condition for integer (30), double(40) and string (60)
            if (filter_value_type == "30" ||
                filter_value_type == "40" ||
                filter_value_type == "60" )
            {
                mp_auto_filter->append_column_match_value(filter_value);
            }
            break;
        }
        case filter_blanks:
            break;
        case filter_nonblanks:
            break;
        case filter_type_invalid:
            break;
    }
}

void gnumeric_filter_context::end_filter()
{
    if (mp_auto_filter)
        mp_auto_filter->commit();
}

void gnumeric_filter_context::end_field()
{
    if (mp_auto_filter)
        mp_auto_filter->commit_column();
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
