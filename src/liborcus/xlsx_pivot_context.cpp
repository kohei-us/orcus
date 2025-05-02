/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "xlsx_pivot_context.hpp"
#include "ooxml_namespace_types.hpp"
#include "ooxml_token_constants.hpp"
#include "xml_context_global.hpp"
#include "session_context.hpp"
#include "xlsx_types.hpp"
#include "impl_utils.hpp"

#include <orcus/measurement.hpp>
#include <orcus/spreadsheet/import_interface.hpp>
#include <orcus/spreadsheet/import_interface_pivot.hpp>
#include <orcus/spreadsheet/import_interface_pivot_table_def.hpp>

#include <optional>
#include <mdds/sorted_string_map.hpp>

namespace ss = orcus::spreadsheet;

namespace orcus {

namespace {

namespace pc_source {

using map_type = mdds::sorted_string_map<xlsx_pivot_cache_def_context::source_type>;

// Keys must be sorted.
constexpr map_type::entry_type entries[] = {
    { "consolidation", xlsx_pivot_cache_def_context::source_type::consolidation },
    { "external",      xlsx_pivot_cache_def_context::source_type::external      },
    { "scenario",      xlsx_pivot_cache_def_context::source_type::scenario      },
    { "worksheet",     xlsx_pivot_cache_def_context::source_type::worksheet     },
};

const map_type& get()
{
    static const map_type map(
        entries, std::size(entries),
        xlsx_pivot_cache_def_context::source_type::unknown);

    return map;
}

} // namespace pc_source

namespace item_type {

using map_type = mdds::sorted_string_map<ss::pivot_field_item_t>;

// Keys must be sorted.
constexpr map_type::entry_type entries[] = {
    { "avg", ss::pivot_field_item_t::average, },
    { "blank", ss::pivot_field_item_t::blank_line, },
    { "count", ss::pivot_field_item_t::count },
    { "countA", ss::pivot_field_item_t::count_numbers },
    { "data", ss::pivot_field_item_t::data },
    { "default", ss::pivot_field_item_t::subtotal_default },
    { "grand", ss::pivot_field_item_t::grand_total },
    { "max", ss::pivot_field_item_t::max },
    { "min", ss::pivot_field_item_t::min },
    { "product", ss::pivot_field_item_t::product },
    { "stdDev", ss::pivot_field_item_t::stddev },
    { "stdDevP", ss::pivot_field_item_t::stddevp },
    { "sum", ss::pivot_field_item_t::sum },
    { "var", ss::pivot_field_item_t::var },
    { "varP", ss::pivot_field_item_t::varp },
};

const map_type& get()
{
    static const map_type map(entries, std::size(entries), ss::pivot_field_item_t::unknown);
    return map;
}

} // namespace item_type

namespace axis_type {

using map_type = mdds::sorted_string_map<ss::pivot_axis_t>;

// Keys must be sorted.
constexpr map_type::entry_type entries[] = {
    { "axisCol", ss::pivot_axis_t::column },
    { "axisPage", ss::pivot_axis_t::page },
    { "axisRow", ss::pivot_axis_t::row },
    { "axisValues", ss::pivot_axis_t::values },
};

const map_type& get()
{
    static const map_type map(entries, std::size(entries), ss::pivot_axis_t::unknown);
    return map;
}

} // namespace axis_type

namespace data_subtotal {

using map_type = mdds::sorted_string_map<ss::pivot_data_subtotal_t>;

// Keys must be sorted.
constexpr map_type::entry_type entries[] = {
    { "average", ss::pivot_data_subtotal_t::average },
    { "count", ss::pivot_data_subtotal_t::count },
    { "countNums", ss::pivot_data_subtotal_t::count_numbers },
    { "max", ss::pivot_data_subtotal_t::max },
    { "min", ss::pivot_data_subtotal_t::min },
    { "product", ss::pivot_data_subtotal_t::product },
    { "stdDev", ss::pivot_data_subtotal_t::stddev },
    { "stdDevp", ss::pivot_data_subtotal_t::stddevp },
    { "sum", ss::pivot_data_subtotal_t::sum },
    { "var", ss::pivot_data_subtotal_t::var },
    { "varp", ss::pivot_data_subtotal_t::varp },
};

const map_type& get()
{
    static const map_type map(entries, std::size(entries), ss::pivot_data_subtotal_t::unknown);
    return map;
}

} // namespace data_subtotal

namespace show_data_as_type {

using map_type = mdds::sorted_string_map<ss::pivot_data_show_data_as_t>;

// Keys must be sorted.
constexpr map_type::entry_type entries[] = {
    { "difference", ss::pivot_data_show_data_as_t::difference },
    { "index", ss::pivot_data_show_data_as_t::index },
    { "normal", ss::pivot_data_show_data_as_t::normal },
    { "percent", ss::pivot_data_show_data_as_t::percent },
    { "percentDiff", ss::pivot_data_show_data_as_t::percent_diff },
    { "percentOfCol", ss::pivot_data_show_data_as_t::percent_of_col },
    { "percentOfRow", ss::pivot_data_show_data_as_t::percent_of_row },
    { "percentOfTotal", ss::pivot_data_show_data_as_t::percent_of_total },
    { "runTotal", ss::pivot_data_show_data_as_t::run_total },
};

const map_type& get()
{
    static const map_type map(entries, std::size(entries), ss::pivot_data_show_data_as_t::unknown);
    return map;
}

} // namespace show_data_as_type

}

xlsx_pivot_cache_def_context::xlsx_pivot_cache_def_context(
    session_context& cxt, const tokens& tokens,
    ss::iface::import_pivot_cache_definition& pcache,
    ss::pivot_cache_id_t pcache_id) :
    xml_context_base(cxt, tokens), m_pcache(pcache), m_pcache_id(pcache_id)
{
    static const xml_element_validator::rule rules[] = {
        // parent element -> child element
        { XMLNS_UNKNOWN_ID, XML_UNKNOWN_TOKEN, NS_ooxml_xlsx, XML_pivotCacheDefinition }, // root element
        { NS_ooxml_xlsx, XML_cacheField, NS_ooxml_xlsx, XML_fieldGroup },
        { NS_ooxml_xlsx, XML_cacheField, NS_ooxml_xlsx, XML_sharedItems },
        { NS_ooxml_xlsx, XML_cacheFields, NS_ooxml_xlsx, XML_cacheField },
        { NS_ooxml_xlsx, XML_cacheSource, NS_ooxml_xlsx, XML_worksheetSource },
        { NS_ooxml_xlsx, XML_discretePr, NS_ooxml_xlsx, XML_x, },
        { NS_ooxml_xlsx, XML_fieldGroup, NS_ooxml_xlsx, XML_discretePr },
        { NS_ooxml_xlsx, XML_fieldGroup, NS_ooxml_xlsx, XML_groupItems },
        { NS_ooxml_xlsx, XML_fieldGroup, NS_ooxml_xlsx, XML_rangePr },
        { NS_ooxml_xlsx, XML_groupItems, NS_ooxml_xlsx, XML_s },
        { NS_ooxml_xlsx, XML_pivotCacheDefinition, NS_ooxml_xlsx, XML_cacheFields },
        { NS_ooxml_xlsx, XML_pivotCacheDefinition, NS_ooxml_xlsx, XML_cacheSource },
        { NS_ooxml_xlsx, XML_reference, NS_ooxml_xlsx, XML_x },
        { NS_ooxml_xlsx, XML_sharedItems, NS_ooxml_xlsx, XML_d },
        { NS_ooxml_xlsx, XML_sharedItems, NS_ooxml_xlsx, XML_e },
        { NS_ooxml_xlsx, XML_sharedItems, NS_ooxml_xlsx, XML_n },
        { NS_ooxml_xlsx, XML_sharedItems, NS_ooxml_xlsx, XML_s },
    };

    init_element_validator(rules, std::size(rules));
}

void xlsx_pivot_cache_def_context::start_element(xmlns_id_t ns, xml_token_t name, const xml_token_attrs_t& attrs)
{
    xml_token_pair_t parent = push_stack(ns, name);
    if (ns != NS_ooxml_xlsx)
        return;

    switch (name)
    {
        case XML_pivotCacheDefinition:
        {
            std::string_view refreshed_by;
            std::string_view rid;
            long record_count = -1;

            for_each(attrs.begin(), attrs.end(),
                [&](const xml_token_attr_t& attr)
                {
                    if (!attr.ns || attr.ns == NS_ooxml_xlsx)
                    {
                        switch (attr.name)
                        {
                            case XML_refreshedBy:
                                refreshed_by = attr.value;
                            break;
                            case XML_recordCount:
                                record_count = to_long(attr.value);
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
                                // relation id for its cache record.
                                rid = attr.value;
                            break;
                            default:
                                ;
                        }
                    }
                }
            );

            if (!rid.empty())
            {
                // The rid string here must be persistent beyond the current
                // context.
                rid = get_session_context().spool.intern(rid).first;

                m_pcache_info.data.insert(
                    opc_rel_extras_t::map_type::value_type(
                        rid, std::make_unique<xlsx_rel_pivot_cache_record_info>(m_pcache_id)));
            }

            break;
        }
        case XML_cacheSource:
        {
            std::string_view source_type_s;

            for_each(attrs.begin(), attrs.end(),
                [&](const xml_token_attr_t& attr)
                {
                    if (attr.ns && attr.ns != NS_ooxml_xlsx)
                        return;

                    switch (attr.name)
                    {
                        case XML_type:
                            m_source_type = pc_source::get().find(attr.value);
                            source_type_s = attr.value;
                            break;
                        default:
                            ;
                    }
                }
            );

            break;
        }
        case XML_worksheetSource:
        {
            if (m_source_type != source_type::worksheet)
                throw xml_structure_error(
                    "worksheetSource element encountered while the source type is not worksheet.");

            std::string_view ref, sheet_name, table_name;

            for_each(attrs.begin(), attrs.end(),
                [&](const xml_token_attr_t& attr)
                {
                    if (attr.ns && attr.ns != NS_ooxml_xlsx)
                        return;

                    switch (attr.name)
                    {
                        case XML_ref:
                            ref = attr.value;
                            break;
                        case XML_sheet:
                            sheet_name = attr.value;
                            break;
                        case XML_name:
                            table_name = attr.value;
                            break;
                        default:
                            ;
                    }
                }
            );

            if (!table_name.empty())
                m_pcache.set_worksheet_source(table_name);
            else
                m_pcache.set_worksheet_source(ref, sheet_name);
            break;
        }
        case XML_cacheFields:
        {
            long field_count = -1;
            if (auto v = get_single_long_attr(attrs, NS_ooxml_xlsx, XML_count); v)
                field_count = *v;
            else
                throw xml_structure_error("failed to get a field count from cacheFields");

            if (field_count < 0)
                throw xml_structure_error("field count of a pivot cache must be positive.");

            m_pcache.set_field_count(field_count);

            break;
        }
        case XML_cacheField:
        {
            std::string_view field_name;
            long numfmt_id = -1;

            for_each(attrs.begin(), attrs.end(),
                [&](const xml_token_attr_t& attr)
                {
                    if (attr.ns && attr.ns != NS_ooxml_xlsx)
                        return;

                    switch (attr.name)
                    {
                        case XML_name:
                            field_name = attr.value;
                        break;
                        case XML_numFmtId:
                            numfmt_id = to_long(attr.value);
                        break;
                        default:
                            ;
                    }
                }
            );

            // TODO : Handle number format ID here.
            m_pcache.set_field_name(field_name);
            break;
        }
        case XML_fieldGroup:
        {
            long group_parent = -1;
            long group_base = -1;

            for_each(attrs.begin(), attrs.end(),
                [&](const xml_token_attr_t& attr)
                {
                    if (attr.ns && attr.ns != NS_ooxml_xlsx)
                        return;

                    switch (attr.name)
                    {
                        case XML_par:
                            group_parent = to_long(attr.value);
                            break;
                        case XML_base:
                            group_base = to_long(attr.value);
                            break;
                        default:
                            ;
                    }
                }
            );

            if (group_base >= 0)
            {
                // This is a group field.
                m_pcache_field_group = m_pcache.start_field_group(group_base);
            }
            break;
        }
        case XML_discretePr:
        {
            long count = -1;

            for_each(attrs.begin(), attrs.end(),
                [&](const xml_token_attr_t& attr)
                {
                    if (attr.ns && attr.ns != NS_ooxml_xlsx)
                        return;

                    switch (attr.name)
                    {
                        case XML_count:
                            count = to_long(attr.value);
                            break;
                        default:
                            ;
                    }
                }
            );

            break;
        }
        case XML_rangePr:
        {
            bool auto_start = true;
            bool auto_end = true;
            double start = 0.0;
            double end = 0.0;
            double interval = 1.0;

            std::optional<date_time_t> start_date;
            std::optional<date_time_t> end_date;

            // Default group-by type appears to be 'range'.
            ss::pivot_cache_group_by_t group_by =
                ss::pivot_cache_group_by_t::range;

            for_each(attrs.begin(), attrs.end(),
                [&](const xml_token_attr_t& attr)
                {
                    if (attr.ns && attr.ns != NS_ooxml_xlsx)
                        return;

                    switch (attr.name)
                    {
                        case XML_autoStart:
                            auto_start = to_bool(attr.value);
                            break;
                        case XML_autoEnd:
                            auto_end = to_bool(attr.value);
                            break;
                        case XML_startNum:
                            start = to_double(attr.value);
                            break;
                        case XML_endNum:
                            end = to_double(attr.value);
                            break;
                        case XML_groupInterval:
                            interval = to_double(attr.value);
                            break;
                        case XML_startDate:
                            start_date = date_time_t::from_chars(attr.value);
                            break;
                        case XML_endDate:
                            end_date = date_time_t::from_chars(attr.value);
                            break;
                        case XML_groupBy:
                            group_by = ss::to_pivot_cache_group_by_enum(attr.value);
                            break;
                        default:
                            ;
                    }
                }
            );

            if (m_pcache_field_group)
            {
                // Pass the values to the interface.
                m_pcache_field_group->set_range_grouping_type(group_by);
                m_pcache_field_group->set_range_auto_start(auto_start);
                m_pcache_field_group->set_range_auto_end(auto_end);
                m_pcache_field_group->set_range_start_number(start);
                m_pcache_field_group->set_range_end_number(end);
                m_pcache_field_group->set_range_interval(interval);

                if (start_date)
                    m_pcache_field_group->set_range_start_date(*start_date);
                if (end_date)
                    m_pcache_field_group->set_range_end_date(*end_date);
            }

            break;
        }
        case XML_sharedItems:
        {
            start_element_shared_items(parent, attrs);
            break;
        }
        case XML_groupItems:
        {
            long count = -1;

            for_each(attrs.begin(), attrs.end(),
                [&](const xml_token_attr_t& attr)
                {
                    if (attr.ns && attr.ns != NS_ooxml_xlsx)
                        return;

                    switch (attr.name)
                    {
                        case XML_count:
                            count = to_long(attr.value);
                            break;
                        default:
                            ;
                    }
                }
            );

            break;
        }
        case XML_s:
        {
            start_element_s(parent, attrs);
            break;
        }
        case XML_n:
        {
            start_element_n(parent, attrs);
            break;
        }
        case XML_d:
        {
            start_element_d(parent, attrs);
            break;
        }
        case XML_e:
        {
            start_element_e(parent, attrs);
            break;
        }
        case XML_x:
        {
            long index = -1;

            for_each(attrs.begin(), attrs.end(),
                [&](const xml_token_attr_t& attr)
                {
                    if (attr.ns && attr.ns != NS_ooxml_xlsx)
                        return;

                    switch (attr.name)
                    {
                        case XML_v:
                            index = to_long(attr.value);
                            break;
                        default:
                            ;
                    }
                }
            );

            if (index < 0)
                throw xml_structure_error("element 'x' without a required attribute 'v'.");

            if (m_pcache_field_group)
                m_pcache_field_group->link_base_to_group_items(index);

            break;
        }
        default:
            warn_unhandled();
    }
}

bool xlsx_pivot_cache_def_context::end_element(xmlns_id_t ns, xml_token_t name)
{
    if (ns == NS_ooxml_xlsx)
    {
        switch (name)
        {
            case XML_pivotCacheDefinition:
            {
                m_pcache.commit();
                break;
            }
            case XML_cacheField:
            {
                m_pcache.commit_field();
                m_pcache_field_group = nullptr;
                break;
            }
            case XML_discretePr:
            {
                break;
            }
            case XML_fieldGroup:
            {
                if (m_pcache_field_group)
                    m_pcache_field_group->commit();
                break;
            }
            case XML_s:
                end_element_s();
                break;
            case XML_n:
                end_element_n();
                break;
            case XML_d:
                end_element_d();
                break;
            case XML_e:
                end_element_e();
                break;
            default:
                ;
        }
    }

    return pop_stack(ns, name);
}

opc_rel_extras_t xlsx_pivot_cache_def_context::pop_rel_extras()
{
    return std::move(m_pcache_info);
}

void xlsx_pivot_cache_def_context::start_element_s(
    const xml_token_pair_t& parent, const xml_token_attrs_t& attrs)
{
    std::string_view value;

    for_each(attrs.begin(), attrs.end(),
        [&](const xml_token_attr_t& attr)
        {
            if (attr.ns && attr.ns != NS_ooxml_xlsx)
                return;

            switch (attr.name)
            {
                case XML_v:
                    value = attr.value;
                break;
                default:
                    ;
            }
        }
    );

    switch (parent.second)
    {
        case XML_sharedItems:
        {
            // regular (non-group) field member name.

            m_field_item_used = true;
            m_pcache.set_field_item_string(value);
            break;
        }
        case XML_groupItems:
        {
            // group field member name.

            m_field_item_used = true;
            if (m_pcache_field_group)
                m_pcache_field_group->set_field_item_string(value);
            break;
        }
        default:
            warn_unhandled();
    }
}

void xlsx_pivot_cache_def_context::end_element_s()
{
    const xml_token_pair_t& parent = get_parent_element();
    if (parent.first != NS_ooxml_xlsx)
        return;

    switch (parent.second)
    {
        case XML_sharedItems:
        {
            if (m_field_item_used)
                m_pcache.commit_field_item();
            break;
        }
        case XML_groupItems:
        {
            if (m_pcache_field_group && m_field_item_used)
                m_pcache_field_group->commit_field_item();
            break;
        }
        default:
            ;
    }
}

void xlsx_pivot_cache_def_context::start_element_n(
    const xml_token_pair_t& parent, const xml_token_attrs_t& attrs)
{
    switch (parent.second)
    {
        case XML_sharedItems:
        {
            // numeric item of a cache field.
            double value = 0.0;
            m_field_item_used = true;

            for_each(attrs.begin(), attrs.end(),
                [&](const xml_token_attr_t& attr)
                {
                    if (attr.ns && attr.ns != NS_ooxml_xlsx)
                        return;

                    switch (attr.name)
                    {
                        case XML_v:
                            value = to_double(attr.value);
                        break;
                        case XML_u:
                            // flag for unused item.
                            m_field_item_used = !to_bool(attr.value);
                        default:
                            ;
                    }
                }
            );

            if (m_field_item_used)
                m_pcache.set_field_item_numeric(value);

            break;
        }
        default:
            warn_unhandled();
    }
}

void xlsx_pivot_cache_def_context::end_element_n()
{
    const xml_token_pair_t& parent = get_parent_element();
    if (parent.first != NS_ooxml_xlsx)
        return;

    switch (parent.second)
    {
        case XML_sharedItems:
        {
            if (m_field_item_used)
                m_pcache.commit_field_item();
            break;
        }
        default:
            ;
    }
}

void xlsx_pivot_cache_def_context::start_element_d(
    const xml_token_pair_t& parent, const xml_token_attrs_t& attrs)
{
    switch (parent.second)
    {
        case XML_sharedItems:
        {
            // date item of a cache field.
            date_time_t dt;
            m_field_item_used = true;

            for_each(attrs.begin(), attrs.end(),
                [&](const xml_token_attr_t& attr)
                {
                    if (attr.ns && attr.ns != NS_ooxml_xlsx)
                        return;

                    switch (attr.name)
                    {
                        case XML_v:
                            dt = date_time_t::from_chars(attr.value);
                        break;
                        case XML_u:
                            // flag for unused item.
                            m_field_item_used = !to_bool(attr.value);
                        default:
                            ;
                    }
                }
            );

            if (m_field_item_used)
                m_pcache.set_field_item_date_time(dt);

            break;
        }
        default:
            ;
    }
}

void xlsx_pivot_cache_def_context::end_element_d()
{
    const xml_token_pair_t& parent = get_parent_element();
    if (parent.first != NS_ooxml_xlsx)
        return;

    switch (parent.second)
    {
        case XML_sharedItems:
        {
            if (m_field_item_used)
                m_pcache.commit_field_item();
            break;
        }
        default:
            ;
    }
}

void xlsx_pivot_cache_def_context::start_element_e(
    const xml_token_pair_t& parent, const xml_token_attrs_t& attrs)
{
    switch (parent.second)
    {
        case XML_sharedItems:
        {
            // error value item of a cache field.
            ss::error_value_t ev = ss::error_value_t::unknown;
            m_field_item_used = true;

            for_each(attrs.begin(), attrs.end(),
                [&](const xml_token_attr_t& attr)
                {
                    if (attr.ns && attr.ns != NS_ooxml_xlsx)
                        return;

                    switch (attr.name)
                    {
                        case XML_v:
                            ev = ss::to_error_value_enum(attr.value);
                        break;
                        case XML_u:
                            // flag for unused item.
                            m_field_item_used = !to_bool(attr.value);
                        default:
                            ;
                    }
                }
            );

            if (m_field_item_used)
                m_pcache.set_field_item_error(ev);

            break;
        }
        default:
            ;
    }
}

void xlsx_pivot_cache_def_context::end_element_e()
{
    const xml_token_pair_t& parent = get_parent_element();
    if (parent.first != NS_ooxml_xlsx)
        return;

    switch (parent.second)
    {
        case XML_sharedItems:
        {
            if (m_field_item_used)
                m_pcache.commit_field_item();
            break;
        }
        default:
            ;
    }
}

void xlsx_pivot_cache_def_context::start_element_shared_items(
    const xml_token_pair_t& parent, const xml_token_attrs_t& attrs)
{
    // If "semi-mixed types" is set, the field contains text values and at
    // least one other type.
    bool semi_mixed_types = true;

    bool has_non_date = true;
    bool has_string = true;
    bool has_blank = false;

    // If "mixed types" is set, the field contains more than one data types.
    bool mixed_types = false;

    bool has_number = false;
    bool has_integer = false;
    bool has_long_text = false;

    long count = -1;
    std::optional<double> min_value;
    std::optional<double> max_value;
    std::optional<date_time_t> min_date;
    std::optional<date_time_t> max_date;

    for_each(attrs.begin(), attrs.end(),
        [&](const xml_token_attr_t& attr)
        {
            if (attr.ns && attr.ns != NS_ooxml_xlsx)
                return;

            switch (attr.name)
            {
                case XML_count:
                    count = to_long(attr.value);
                    break;
                case XML_containsMixedTypes:
                    mixed_types = to_bool(attr.value);
                    break;
                case XML_containsSemiMixedTypes:
                    semi_mixed_types = to_bool(attr.value);
                    break;
                case XML_containsNonDate:
                    has_non_date = to_bool(attr.value);
                    break;
                case XML_containsString:
                    has_string = to_bool(attr.value);
                    break;
                case XML_containsBlank:
                    has_blank = to_bool(attr.value);
                    break;
                case XML_containsNumber:
                    has_number = to_bool(attr.value);
                    break;
                case XML_containsInteger:
                    has_integer = to_bool(attr.value);
                    break;
                case XML_minValue:
                    min_value = to_double(attr.value);
                    break;
                case XML_maxValue:
                    max_value = to_double(attr.value);
                    break;
                case XML_minDate:
                    min_date = date_time_t::from_chars(attr.value);
                    break;
                case XML_maxDate:
                    max_date = date_time_t::from_chars(attr.value);
                    break;
                case XML_longText:
                    has_long_text = to_bool(attr.value);
                    break;
                default:
                    ;
            }
        }
    );

    if (min_value)
        m_pcache.set_field_min_value(*min_value);

    if (max_value)
        m_pcache.set_field_max_value(*max_value);

    if (min_date)
        m_pcache.set_field_min_date(*min_date);

    if (max_date)
        m_pcache.set_field_max_date(*max_date);
}

xlsx_pivot_cache_rec_context::xlsx_pivot_cache_rec_context(
    session_context& cxt, const tokens& tokens,
    ss::iface::import_pivot_cache_records& pc_records) :
    xml_context_base(cxt, tokens),
    m_pc_records(pc_records)
{
    static const xml_element_validator::rule rules[] = {
        // parent element -> child element
        { XMLNS_UNKNOWN_ID, XML_UNKNOWN_TOKEN, NS_ooxml_xlsx, XML_pivotCacheRecords }, // root element
        { NS_ooxml_xlsx, XML_pivotCacheRecords, NS_ooxml_xlsx, XML_r },
        { NS_ooxml_xlsx, XML_r, NS_ooxml_xlsx, XML_b },
        { NS_ooxml_xlsx, XML_r, NS_ooxml_xlsx, XML_d },
        { NS_ooxml_xlsx, XML_r, NS_ooxml_xlsx, XML_e },
        { NS_ooxml_xlsx, XML_r, NS_ooxml_xlsx, XML_m },
        { NS_ooxml_xlsx, XML_r, NS_ooxml_xlsx, XML_n },
        { NS_ooxml_xlsx, XML_r, NS_ooxml_xlsx, XML_s },
        { NS_ooxml_xlsx, XML_r, NS_ooxml_xlsx, XML_x },
    };

    init_element_validator(rules, std::size(rules));
}

void xlsx_pivot_cache_rec_context::start_element(xmlns_id_t ns, xml_token_t name, const xml_token_attrs_t& attrs)
{
    push_stack(ns, name);

    if (ns != NS_ooxml_xlsx)
        return;

    switch (name)
    {
        case XML_pivotCacheRecords:
        {
            if (auto count = get_single_long_attr(attrs, NS_ooxml_xlsx, XML_count); count)
                m_pc_records.set_record_count(*count);
            break;
        }
        case XML_r: // record
            break;
        case XML_s: // character value
        {
            std::string_view cv = get_single_attr(attrs, NS_ooxml_xlsx, XML_v);
            m_pc_records.append_record_value_character(cv);
            break;
        }
        case XML_x: // shared item index
        {
            if (auto v = get_single_long_attr(attrs, NS_ooxml_xlsx, XML_v); v)
                m_pc_records.append_record_value_shared_item(*v);
            else
                throw xml_structure_error("failed to get a record value shared item index");

            break;
        }
        case XML_n: // numeric
        {
            auto val = get_single_double_attr(attrs, NS_ooxml_xlsx, XML_v);
            if (!val)
                throw xml_structure_error("failed to get a numeric record value in pivot cache record");

            m_pc_records.append_record_value_numeric(*val);
            break;
        }
        case XML_e: // error value
        {
            std::string_view cv = get_single_attr(attrs, NS_ooxml_xlsx, XML_v);
            (void)cv;
            break;
        }
        case XML_b: // boolean
        case XML_d: // date time
        case XML_m: // no value
        default:
            warn_unhandled();
    }
}

bool xlsx_pivot_cache_rec_context::end_element(xmlns_id_t ns, xml_token_t name)
{
    if (ns == NS_ooxml_xlsx)
    {
        switch (name)
        {
            case XML_pivotCacheRecords:
                m_pc_records.commit();
                break;
            case XML_r: // record
                m_pc_records.commit_record();
                break;
            default:
                ;
        }
    }

    return pop_stack(ns, name);
}

xlsx_pivot_table_context::xlsx_pivot_table_context(
    session_context& cxt, const tokens& tokens,
    ss::iface::import_pivot_table_definition& xpt,
    ss::iface::import_reference_resolver& resolver) :
    xml_context_base(cxt, tokens), m_xpt(xpt), m_resolver(resolver)
{
    static const xml_element_validator::rule rules[] = {
        // parent element -> child element
        { XMLNS_UNKNOWN_ID, XML_UNKNOWN_TOKEN, NS_ooxml_xlsx, XML_pivotTableDefinition }, // root element
        { NS_ooxml_xlsx, XML_colFields, NS_ooxml_xlsx, XML_field },
        { NS_ooxml_xlsx, XML_colItems, NS_ooxml_xlsx, XML_i },
        { NS_ooxml_xlsx, XML_dataFields, NS_ooxml_xlsx, XML_dataField },
        { NS_ooxml_xlsx, XML_i, NS_ooxml_xlsx, XML_x },
        { NS_ooxml_xlsx, XML_items, NS_ooxml_xlsx, XML_item },
        { NS_ooxml_xlsx, XML_pageFields, NS_ooxml_xlsx, XML_pageField },
        { NS_ooxml_xlsx, XML_pivotField, NS_ooxml_xlsx, XML_items },
        { NS_ooxml_xlsx, XML_pivotFields, NS_ooxml_xlsx, XML_pivotField },
        { NS_ooxml_xlsx, XML_pivotTableDefinition, NS_ooxml_xlsx, XML_colFields },
        { NS_ooxml_xlsx, XML_pivotTableDefinition, NS_ooxml_xlsx, XML_colItems },
        { NS_ooxml_xlsx, XML_pivotTableDefinition, NS_ooxml_xlsx, XML_dataFields },
        { NS_ooxml_xlsx, XML_pivotTableDefinition, NS_ooxml_xlsx, XML_location },
        { NS_ooxml_xlsx, XML_pivotTableDefinition, NS_ooxml_xlsx, XML_pageFields },
        { NS_ooxml_xlsx, XML_pivotTableDefinition, NS_ooxml_xlsx, XML_pivotFields },
        { NS_ooxml_xlsx, XML_pivotTableDefinition, NS_ooxml_xlsx, XML_pivotTableStyleInfo },
        { NS_ooxml_xlsx, XML_pivotTableDefinition, NS_ooxml_xlsx, XML_rowFields },
        { NS_ooxml_xlsx, XML_pivotTableDefinition, NS_ooxml_xlsx, XML_rowItems },
        { NS_ooxml_xlsx, XML_rowFields, NS_ooxml_xlsx, XML_field },
        { NS_ooxml_xlsx, XML_rowItems, NS_ooxml_xlsx, XML_i },
    };

    init_element_validator(rules, std::size(rules));
}

void xlsx_pivot_table_context::start_element(xmlns_id_t ns, xml_token_t name, const xml_token_attrs_t& attrs)
{
    xml_token_pair_t parent = push_stack(ns, name);
    if (ns == NS_ooxml_xlsx)
    {
        switch (name)
        {
            case XML_pivotTableDefinition:
            {
                start_pivot_table_definition(attrs);
                break;
            }
            case XML_location:
            {
                start_location(attrs);
                break;
            }
            case XML_pivotFields:
            {
                start_pivot_fields(attrs);
                break;
            }
            case XML_pivotField:
            {
                start_pivot_field(attrs);
                break;
            }
            case XML_items:
            {
                start_items(attrs);
                break;
            }
            case XML_item:
            {
                start_item(attrs);
                break;
            }
            case XML_rowFields:
            {
                start_row_fields(attrs);
                break;
            }
            case XML_colFields:
            {
                start_col_fields(attrs);
                break;
            }
            case XML_pageFields:
            {
                start_page_fields(attrs);
                break;
            }
            case XML_pageField:
            {
                start_page_field(attrs);
                break;
            }
            case XML_field:
            {
                start_field(attrs);
                break;
            }
            case XML_dataFields:
            {
                start_data_fields(attrs);
                break;
            }
            case XML_dataField:
            {
                start_data_field(attrs);
                break;
            }
            case XML_rowItems:
            {
                start_row_items(attrs);
                break;
            }
            case XML_colItems:
            {
                start_col_items(attrs);
                break;
            }
            case XML_i:
            {
                start_i(attrs);
                break;
            }
            case XML_x:
            {
                start_x(attrs, parent);
                break;
            }
            case XML_pivotTableStyleInfo:
            {
                start_pivot_table_style_info(attrs);
                break;
            }
            default:
                warn_unhandled();
        }
    }
    else
        warn_unhandled();
}

bool xlsx_pivot_table_context::end_element(xmlns_id_t ns, xml_token_t name)
{
    if (ns == NS_ooxml_xlsx)
    {
        switch (name)
        {
            case XML_pivotTableDefinition:
            {
                m_xpt.commit();
                break;
            }
            case XML_pivotFields:
            {
                assert(m_pivot_fields);
                m_pivot_fields->commit();
                m_pivot_fields = nullptr;
                break;
            }
            case XML_pivotField:
            {
                assert(m_pivot_field);
                m_pivot_field->commit();
                m_pivot_field = nullptr;
                break;
            }
            case XML_rowFields:
            {
                assert(m_rc_fields);
                m_rc_fields->commit();
                m_rc_fields = nullptr;
                break;
            }
            case XML_colFields:
            {
                assert(m_rc_fields);
                m_rc_fields->commit();
                m_rc_fields = nullptr;
                break;
            }
            case XML_pageFields:
            {
                assert(m_page_fields);
                m_page_fields->commit();
                m_page_fields = nullptr;
                break;
            }
            case XML_pageField:
            {
                assert(m_page_field);
                m_page_field->commit();
                m_page_field = nullptr;
                break;
            }
            case XML_dataFields:
            {
                assert(m_data_fields);
                m_data_fields->commit();
                m_data_fields = nullptr;
                break;
            }
            case XML_dataField:
            {
                assert(m_data_field);
                m_data_field->commit();
                m_data_field = nullptr;
                break;
            }
            case XML_rowItems:
            {
                assert(m_rc_items);
                m_rc_items->commit();
                m_rc_items = nullptr;
                break;
            }
            case XML_colItems:
            {
                assert(m_rc_items);
                m_rc_items->commit();
                m_rc_items = nullptr;
                break;
            }
            case XML_i:
            {
                assert(m_rc_item);
                m_rc_item->commit();
                m_rc_item = nullptr;
                break;
            }
        }
    }

    return pop_stack(ns, name);
}

void xlsx_pivot_table_context::start_pivot_table_definition(const xml_token_attrs_t& attrs)
{
    for (const xml_token_attr_t& attr : attrs)
    {
        if (attr.ns)
            continue;

        switch (attr.name)
        {
            case XML_name:
            {
                m_xpt.set_name(attr.value);
                break;
            }
            case XML_cacheId:
            {
                if (auto v = to_long_checked(attr.value); v)
                    m_xpt.set_cache_id(*v);
                break;
            }
        }
    }
}

void xlsx_pivot_table_context::start_location(const xml_token_attrs_t& attrs)
{
    for (const xml_token_attr_t& attr : attrs)
    {
        if (attr.ns)
            continue;

        switch (attr.name)
        {
            case XML_ref:
            {
                auto range = ss::to_rc_range(m_resolver.resolve_range(attr.value));
                m_xpt.set_range(range);
                break;
            }
        }
    }
}

void xlsx_pivot_table_context::start_pivot_fields(const xml_token_attrs_t& attrs)
{
    m_pivot_fields = m_xpt.start_pivot_fields();
    ENSURE_INTERFACE(m_pivot_fields, import_pivot_fields);

    // pivotFields and its child elements represent the visual
    // appearances of the fields inside pivot table.
    if (auto count = get_single_long_attr(attrs, NS_ooxml_xlsx, XML_count); count)
        m_pivot_fields->set_count(*count);
}

void xlsx_pivot_table_context::start_pivot_field(const xml_token_attrs_t& attrs)
{
    assert(m_pivot_fields);
    m_pivot_field = m_pivot_fields->start_pivot_field();
    ENSURE_INTERFACE(m_pivot_field, import_pivot_field);

    for (const xml_token_attr_t& attr : attrs)
    {
        if (attr.ns)
            continue;

        switch (attr.name)
        {
            case XML_axis:
            {
                auto v = axis_type::get().find(attr.value);
                if (v == ss::pivot_axis_t::unknown)
                {
                    std::ostringstream os;
                    os << "unrecognized pivot aixs type: '" << attr.value << "'";
                    warn(os.str());
                }

                m_pivot_field->set_axis(v);
                break;
            }
        }
    }
}

void xlsx_pivot_table_context::start_items(const xml_token_attrs_t& attrs)
{
    if (auto count = get_single_long_attr(attrs, NS_ooxml_xlsx, XML_count); count)
    {
        assert(m_pivot_field);
        m_pivot_field->set_item_count(*count);
    }
}

void xlsx_pivot_table_context::start_item(const xml_token_attrs_t& attrs)
{
    assert(m_pivot_field);

    std::optional<long> index;
    std::optional<ss::pivot_field_item_t> type;
    bool hidden = false;

    for (const xml_token_attr_t& attr : attrs)
    {
        if (attr.ns)
            continue;

        switch (attr.name)
        {
            case XML_x:
            {
                // field item index as defined in the pivot cache.
                if (index = to_long_checked(attr.value); !index)
                {
                    std::ostringstream os;
                    os << "invalid index value of pivot field item: '" << attr.value << "'";
                    warn(os.str());
                }
                break;
            }
            case XML_t:
            {
                // When the <item> element has attribute 't', it's subtotal or
                // some sort of function item.  See 3.18.45 ST_ItemType
                // (PivotItem Type) for possible values.

                type = item_type::get().find(attr.value);
                if (*type == ss::pivot_field_item_t::unknown)
                {
                    std::ostringstream os;
                    os << "unrecognized pivot field item type: '" << attr.value << "'";
                    warn(os.str());
                }
                break;
            }
            case XML_h:
            {
                hidden = to_bool(attr.value);
                break;
            }
        }
    }

    if (index)
        m_pivot_field->append_item(*index, hidden);
    else if (type)
        m_pivot_field->append_item(*type);
    else
        throw xml_structure_error("pivot field item is missing a required attribute");
}

void xlsx_pivot_table_context::start_row_fields(const xml_token_attrs_t& attrs)
{
    m_rc_fields = m_xpt.start_row_fields();
    ENSURE_INTERFACE(m_rc_fields, import_pivot_rc_fields);

    for (const xml_token_attr_t& attr : attrs)
    {
        if (attr.ns)
            continue;

        switch (attr.name)
        {
            case XML_count:
            {
                m_rc_fields->set_count(to_long(attr.value));
                break;
            }
        }
    }
}

void xlsx_pivot_table_context::start_col_fields(const xml_token_attrs_t& attrs)
{
    m_rc_fields = m_xpt.start_column_fields();
    ENSURE_INTERFACE(m_rc_fields, import_pivot_rc_fields);

    for (const xml_token_attr_t& attr : attrs)
    {
        if (attr.ns)
            continue;

        switch (attr.name)
        {
            case XML_count:
            {
                m_rc_fields->set_count(to_long(attr.value));
                break;
            }
        }
    }
}

void xlsx_pivot_table_context::start_page_fields(const xml_token_attrs_t& attrs)
{
    m_page_fields = m_xpt.start_page_fields();
    ENSURE_INTERFACE(m_page_fields, import_pivot_page_fields);

    for (const xml_token_attr_t& attr : attrs)
    {
        if (attr.ns)
            continue;

        switch (attr.name)
        {
            case XML_count:
            {
                if (auto v = to_long_checked(attr.value); v)
                    m_page_fields->set_count(*v);
                break;
            }
        }
    }
}

void xlsx_pivot_table_context::start_page_field(const xml_token_attrs_t& attrs)
{
    assert(m_page_fields);
    m_page_field = m_page_fields->start_page_field();
    ENSURE_INTERFACE(m_page_field, import_pivot_page_field);

    for (const xml_token_attr_t& attr : attrs)
    {
        if (attr.ns)
            continue;

        switch (attr.name)
        {
            case XML_fld:
            {
                if (auto v = to_long_checked(attr.value); v)
                    m_page_field->set_field(*v);
                break;
            }
            case XML_item:
            {
                if (auto v = to_long_checked(attr.value); v)
                    m_page_field->set_item(*v);
                break;
            }
        }
    }
}

void xlsx_pivot_table_context::start_field(const xml_token_attrs_t& attrs)
{
    assert(m_rc_fields);

    // Index into the list of <pivotField> collection which is given earlier
    // under the <pivotFields> element.  The value of -2 represents a special
    // field that displays the list of data fields when the pivot table contains
    // more than one data field.

    long x = 0; // defaults to 0

    for (const xml_token_attr_t& attr : attrs)
    {
        if (attr.ns)
            continue;

        switch (attr.name)
        {
            case XML_x:
            {
                if (auto v = to_long_checked(attr.value); v)
                    x = *v;
                break;
            }
        }
    }

    if (x == -2)
        m_rc_fields->append_data_field();
    else
        m_rc_fields->append_field(x);
}

void xlsx_pivot_table_context::start_data_fields(const xml_token_attrs_t& attrs)
{
    m_data_fields = m_xpt.start_data_fields();
    ENSURE_INTERFACE(m_data_fields, import_pivot_data_fields);

    for (const xml_token_attr_t& attr : attrs)
    {
        if (attr.ns)
            continue;

        switch (attr.name)
        {
            case XML_count:
            {
                if (auto v = to_long_checked(attr.value); v)
                    m_data_fields->set_count(*v);
                break;
            }
        }
    }
}

void xlsx_pivot_table_context::start_data_field(const xml_token_attrs_t& attrs)
{
    assert(m_data_fields);
    m_data_field = m_data_fields->start_data_field();
    ENSURE_INTERFACE(m_data_field, import_pivot_data_field);

    ss::pivot_data_show_data_as_t show_data_as = ss::pivot_data_show_data_as_t::unknown;
    std::size_t base_field = 0;
    std::size_t base_item = 0;

    for (const xml_token_attr_t& attr : attrs)
    {
        if (attr.ns)
            continue;

        switch (attr.name)
        {
            case XML_name:
            {
                m_data_field->set_name(attr.value);
                break;
            }
            case XML_fld:
            {
                if (auto v = to_long_checked(attr.value); v)
                    m_data_field->set_field(*v);
                break;
            }
            case XML_baseField:
            {
                if (auto v = to_long_checked(attr.value); v)
                    base_field = *v;
                break;
            }
            case XML_baseItem:
            {
                if (auto v = to_long_checked(attr.value); v)
                    base_item = *v;
                break;
            }
            case XML_subtotal:
            {
                if (auto v = data_subtotal::get().find(attr.value); v != ss::pivot_data_subtotal_t::unknown)
                    m_data_field->set_subtotal_function(v);
                break;
            }
            case XML_showDataAs:
            {
                show_data_as = show_data_as_type::get().find(attr.value);
                break;
            }
        }
    }

    if (show_data_as != ss::pivot_data_show_data_as_t::unknown)
        m_data_field->set_show_data_as(show_data_as, base_field, base_item);
}

void xlsx_pivot_table_context::start_row_items(const xml_token_attrs_t& attrs)
{
    // <rowItems> structure describes the displayed content of cells in the row
    // field area.  Each <i> child element represents a single row.

    m_rc_items = m_xpt.start_row_items();
    ENSURE_INTERFACE(m_rc_items, import_pivot_rc_items);

    for (const xml_token_attr_t& attr : attrs)
    {
        if (attr.ns)
            continue;

        switch (attr.name)
        {
            case XML_count:
            {
                if (auto v = to_long_checked(attr.value); v)
                    m_rc_items->set_count(*v);
                break;
            }
        }
    }
}

void xlsx_pivot_table_context::start_col_items(const xml_token_attrs_t& attrs)
{
    m_rc_items = m_xpt.start_col_items();
    ENSURE_INTERFACE(m_rc_items, import_pivot_rc_items);

    for (const xml_token_attr_t& attr : attrs)
    {
        if (attr.ns)
            continue;

        switch (attr.name)
        {
            case XML_count:
            {
                if (auto v = to_long_checked(attr.value); v)
                    m_rc_items->set_count(*v);
                break;
            }
        }
    }
}

void xlsx_pivot_table_context::start_i(const xml_token_attrs_t& attrs)
{
    assert(m_rc_items);
    m_rc_item = m_rc_items->start_item();
    ENSURE_INTERFACE(m_rc_item, import_pivot_rc_item);

    std::optional<long> data_item;
    ss::pivot_field_item_t item_type = ss::pivot_field_item_t::data; // data by default
    long repeat = 0;

    for (const xml_token_attr_t& attr : attrs)
    {
        if (attr.ns)
            continue;

        switch (attr.name)
        {
            case XML_t:
            {
                // total or subtotal function type.
                item_type = item_type::get().find(attr.value);
                if (item_type == ss::pivot_field_item_t::unknown)
                {
                    std::ostringstream os;
                    os << "xlsx_pivot_table_context::start_i: unknown subtotal function type: '" << attr.value << "'";
                    throw xml_structure_error(os.str());
                }
                break;
            }
            case XML_r:
            {
                // "repeated item count" basically is the number of blank cells
                // that repeat prior to the non-empty cell in the same
                // row/column group
                if (auto v = to_long_checked(attr.value); v)
                    repeat = *v;
                break;
            }
            case XML_i:
            {
                // zero-based data item index in case of multiple data items in
                // a data field.
                if (auto v = to_long_checked(attr.value); v)
                    data_item = *v;
                break;
            }
        }
    }

    m_rc_item->set_repeat_items(repeat);
    m_rc_item->set_item_type(item_type);

    if (data_item)
        m_rc_item->set_data_item(*data_item);
}

void xlsx_pivot_table_context::start_x(const xml_token_attrs_t& attrs, const xml_token_pair_t& parent)
{
    if (parent.second == XML_i)
    {
        assert(m_rc_item);

        long item_index = 0; // 0 by default

        for (const xml_token_attr_t& attr : attrs)
        {
            if (attr.ns)
                continue;

            switch (attr.name)
            {
                case XML_v:
                {
                    if (auto v = to_long_checked(attr.value); v)
                        item_index = *v;
                    else
                    {
                        std::ostringstream os;
                        os << "xlsx_pivot_table_context::start_x: failed to parse '" << attr.value << "' as integral value";
                        throw xml_structure_error(os.str());
                    }
                    break;
                }
            }
        }

        m_rc_item->append_index(item_index);
    }
}

void xlsx_pivot_table_context::start_pivot_table_style_info(const xml_token_attrs_t& attrs)
{
    for (const xml_token_attr_t& attr : attrs)
    {
        if (attr.ns && attr.ns != NS_ooxml_xlsx)
            continue;

        switch (attr.name)
        {
            case XML_name:
                break;
            case XML_showRowHeaders:
                break;
            case XML_showColHeaders:
                break;
            case XML_showRowStripes:
                break;
            case XML_showColStripes:
                break;
            case XML_showLastColumn:
                break;
        }
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
