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

#include "orcus/measurement.hpp"
#include "orcus/spreadsheet/import_interface_pivot.hpp"

#include <iostream>
#include <optional>
#include <mdds/sorted_string_map.hpp>

using namespace std;

namespace orcus {

namespace {

typedef mdds::sorted_string_map<xlsx_pivot_cache_def_context::source_type> pc_source_type;

// Keys must be sorted.
pc_source_type::entry pc_source_entries[] = {
    { ORCUS_ASCII("consolidation"), xlsx_pivot_cache_def_context::source_type::consolidation },
    { ORCUS_ASCII("external"),      xlsx_pivot_cache_def_context::source_type::external      },
    { ORCUS_ASCII("scenario"),      xlsx_pivot_cache_def_context::source_type::scenario      },
    { ORCUS_ASCII("worksheet"),     xlsx_pivot_cache_def_context::source_type::worksheet     },
};

const pc_source_type& get_pc_source_map()
{
    static pc_source_type source_map(
        pc_source_entries,
        sizeof(pc_source_entries)/sizeof(pc_source_entries[0]),
        xlsx_pivot_cache_def_context::source_type::unknown);

    return source_map;
}

}

xlsx_pivot_cache_def_context::xlsx_pivot_cache_def_context(
    session_context& cxt, const tokens& tokens,
    spreadsheet::iface::import_pivot_cache_definition& pcache,
    spreadsheet::pivot_cache_id_t pcache_id) :
    xml_context_base(cxt, tokens), m_pcache(pcache), m_pcache_id(pcache_id) {}

xml_context_base* xlsx_pivot_cache_def_context::create_child_context(xmlns_id_t /*ns*/, xml_token_t /*name*/)
{
    return nullptr;
}

void xlsx_pivot_cache_def_context::end_child_context(xmlns_id_t /*ns*/, xml_token_t /*name*/, xml_context_base* /*child*/)
{
}

void xlsx_pivot_cache_def_context::start_element(xmlns_id_t ns, xml_token_t name, const::std::vector<xml_token_attr_t>& attrs)
{
    xml_token_pair_t parent = push_stack(ns, name);
    if (ns != NS_ooxml_xlsx)
        return;

    switch (name)
    {
        case XML_pivotCacheDefinition:
        {
            xml_element_expected(parent, XMLNS_UNKNOWN_ID, XML_UNKNOWN_TOKEN);

            pstring refreshed_by;
            pstring rid;
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

            if (get_config().debug)
            {
                cout << "---" << endl;
                cout << "pivot cache definition" << endl;
                cout << "refreshed by: " << refreshed_by << endl;
                cout << "record count: " << record_count << endl;
                cout << "rid: " << rid << endl;
            }

            if (!rid.empty())
            {
                // The rid string here must be persistent beyond the current
                // context.
                rid = get_session_context().m_string_pool.intern(rid).first;

                m_pcache_info.data.insert(
                    opc_rel_extras_t::map_type::value_type(
                        rid, std::make_unique<xlsx_rel_pivot_cache_record_info>(m_pcache_id)));
            }

            break;
        }
        case XML_cacheSource:
        {
            xml_element_expected(parent, NS_ooxml_xlsx, XML_pivotCacheDefinition);

            pstring source_type_s;

            for_each(attrs.begin(), attrs.end(),
                [&](const xml_token_attr_t& attr)
                {
                    if (attr.ns && attr.ns != NS_ooxml_xlsx)
                        return;

                    switch (attr.name)
                    {
                        case XML_type:
                            m_source_type =
                                get_pc_source_map().find(attr.value.data(), attr.value.size());

                            source_type_s = attr.value;
                            break;
                        default:
                            ;
                    }
                }
            );

            if (get_config().debug)
                cout << "type: " << source_type_s << endl;

            break;
        }
        case XML_worksheetSource:
        {
            xml_element_expected(parent, NS_ooxml_xlsx, XML_cacheSource);
            if (m_source_type != source_type::worksheet)
                throw xml_structure_error(
                    "worksheetSource element encountered while the source type is not worksheet.");

            pstring ref, sheet_name, table_name;

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

            if (get_config().debug)
            {
                cout << "table: " << table_name << endl;
                cout << "ref: " << ref << endl;
                cout << "sheet: " << sheet_name << endl;
            }

            if (!table_name.empty())
                m_pcache.set_worksheet_source(table_name);
            else
                m_pcache.set_worksheet_source(ref, sheet_name);
            break;
        }
        case XML_cacheFields:
        {
            xml_element_expected(parent, NS_ooxml_xlsx, XML_pivotCacheDefinition);
            single_long_attr_getter func(NS_ooxml_xlsx, XML_count);
            long field_count = for_each(attrs.begin(), attrs.end(), func).get_value();

            if (get_config().debug)
                cout << "field count: " << field_count << endl;

            if (field_count < 0)
                throw xml_structure_error("field count of a pivot cache must be positive.");

            m_pcache.set_field_count(field_count);
            break;
        }
        case XML_cacheField:
        {
            xml_element_expected(parent, NS_ooxml_xlsx, XML_cacheFields);

            pstring field_name;
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

            if (get_config().debug)
            {
                cout << "* name: " << field_name << endl;
                cout << "  number format id: " << numfmt_id << endl;
            }
            break;
        }
        case XML_fieldGroup:
        {
            xml_element_expected(parent, NS_ooxml_xlsx, XML_cacheField);
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

            if (get_config().debug)
            {
                if (group_parent >= 0)
                    cout << "  * group parent index: " << group_parent << endl;
                if (group_base >= 0)
                    cout << "  * group base index: " << group_base << endl;
            }

            if (group_base >= 0)
            {
                // This is a group field.
                m_pcache_field_group = m_pcache.create_field_group(group_base);
            }
            break;
        }
        case XML_discretePr:
        {
            xml_element_expected(parent, NS_ooxml_xlsx, XML_fieldGroup);

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

            if (get_config().debug)
                cout << "  * group child member count: " << count << endl;

            break;
        }
        case XML_rangePr:
        {
            xml_element_expected(parent, NS_ooxml_xlsx, XML_fieldGroup);

            bool auto_start = true;
            bool auto_end = true;
            double start = 0.0;
            double end = 0.0;
            double interval = 1.0;

            std::optional<date_time_t> start_date;
            std::optional<date_time_t> end_date;

            // Default group-by type appears to be 'range'.
            spreadsheet::pivot_cache_group_by_t group_by =
                spreadsheet::pivot_cache_group_by_t::range;

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
                            start_date = to_date_time(attr.value);
                            break;
                        case XML_endDate:
                            end_date = to_date_time(attr.value);
                            break;
                        case XML_groupBy:
                            group_by = spreadsheet::to_pivot_cache_group_by_enum(attr.value);
                            break;
                        default:
                            ;
                    }
                }
            );

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

            if (get_config().debug)
            {
                cout << "  auto start: " << auto_start << endl;
                cout << "  auto end: " << auto_end << endl;
                cout << "  start: " << start << endl;
                cout << "  end: " << end << endl;
                cout << "  interval: " << interval << endl;

                if (start_date)
                    cout << "start date: " << *start_date << endl;
                if (end_date)
                    cout << "end date: " << *end_date << endl;
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
            xml_element_expected(parent, NS_ooxml_xlsx, XML_fieldGroup);

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

            if (get_config().debug)
                cout << "  * group member count: " << count << endl;

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
            const xml_elem_set_t expected = {
                { NS_ooxml_xlsx, XML_discretePr },
                { NS_ooxml_xlsx, XML_reference },
            };
            xml_element_expected(parent, expected);

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

            if (get_config().debug)
                cout << "    * index = " << index << endl;

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

void xlsx_pivot_cache_def_context::characters(std::string_view /*str*/, bool /*transient*/)
{
}

opc_rel_extras_t xlsx_pivot_cache_def_context::pop_rel_extras()
{
    return std::move(m_pcache_info);
}

void xlsx_pivot_cache_def_context::start_element_s(
    const xml_token_pair_t& parent, const std::vector<xml_token_attr_t>& attrs)
{
    if (parent.first != NS_ooxml_xlsx)
    {
        warn_unhandled();
        return;
    }

    pstring value;

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

            if (get_config().debug)
                cout << "    * field member: " << value << endl;

            m_field_item_used = true;
            m_pcache.set_field_item_string(value);
            break;
        }
        case XML_groupItems:
        {
            // group field member name.

            if (get_config().debug)
                cout << "    * group field member: " << value << endl;

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
    const xml_token_pair_t& parent, const std::vector<xml_token_attr_t>& attrs)
{
    if (parent.first != NS_ooxml_xlsx)
    {
        warn_unhandled();
        return;
    }

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

            if (get_config().debug)
            {
                cout << "  * n: " << value;
                if (!m_field_item_used)
                    cout << " (unused)";
                cout << endl;

            }

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
    const xml_token_pair_t& parent, const std::vector<xml_token_attr_t>& attrs)
{
    if (parent.first != NS_ooxml_xlsx)
    {
        warn_unhandled();
        return;
    }

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
                            dt = to_date_time(attr.value);
                        break;
                        case XML_u:
                            // flag for unused item.
                            m_field_item_used = !to_bool(attr.value);
                        default:
                            ;
                    }
                }
            );

            if (get_config().debug)
            {
                cout << "  * d: " << dt;
                if (!m_field_item_used)
                    cout << " (unused)";
                cout << endl;

            }

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
    const xml_token_pair_t& parent, const std::vector<xml_token_attr_t>& attrs)
{
    if (parent.first != NS_ooxml_xlsx)
    {
        warn_unhandled();
        return;
    }

    switch (parent.second)
    {
        case XML_sharedItems:
        {
            // error value item of a cache field.
            spreadsheet::error_value_t ev = spreadsheet::error_value_t::unknown;
            m_field_item_used = true;

            for_each(attrs.begin(), attrs.end(),
                [&](const xml_token_attr_t& attr)
                {
                    if (attr.ns && attr.ns != NS_ooxml_xlsx)
                        return;

                    switch (attr.name)
                    {
                        case XML_v:
                            ev = spreadsheet::to_error_value_enum(attr.value);
                        break;
                        case XML_u:
                            // flag for unused item.
                            m_field_item_used = !to_bool(attr.value);
                        default:
                            ;
                    }
                }
            );

            if (get_config().debug)
            {
                cout << "  * e: " << ev;
                if (!m_field_item_used)
                    cout << " (unused)";
                cout << endl;
            }

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
    const xml_token_pair_t& parent, const std::vector<xml_token_attr_t>& attrs)
{
    xml_element_expected(parent, NS_ooxml_xlsx, XML_cacheField);

    // If "semi-mixed types" is set, the field contains text values and at
    // least one other type.
    bool semi_mixed_types = true;

    bool has_non_date = true;
    bool has_date = false;
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
                    min_date = to_date_time(attr.value);
                    break;
                case XML_maxDate:
                    max_date = to_date_time(attr.value);
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

    if (get_config().debug)
    {
        cout << "  contains semi-mixed types: " << semi_mixed_types << endl;
        cout << "  contains non-date: " << has_non_date << endl;
        cout << "  contains date: " << has_date << endl;
        cout << "  contains string: " << has_string << endl;
        cout << "  contains blank: " << has_blank << endl;
        cout << "  contains mixed types: " << mixed_types << endl;
        cout << "  contains number: " << has_number << endl;
        cout << "  contains integer: " << has_integer << endl;
        cout << "  contains long text: " << has_long_text << endl;
        cout << "  count: " << count << endl;

        if (min_value)
            cout << "  min value: " << *min_value << endl;
        if (max_value)
            cout << "  max value: " << *max_value << endl;
        if (min_date)
            cout << "  min date: " << *min_date << endl;
        if (max_date)
            cout << "  max date: " << *max_date << endl;
    }
}

xlsx_pivot_cache_rec_context::xlsx_pivot_cache_rec_context(
    session_context& cxt, const tokens& tokens,
    spreadsheet::iface::import_pivot_cache_records& pc_records) :
    xml_context_base(cxt, tokens),
    m_pc_records(pc_records) {}

xml_context_base* xlsx_pivot_cache_rec_context::create_child_context(xmlns_id_t /*ns*/, xml_token_t /*name*/)
{
    return nullptr;
}

void xlsx_pivot_cache_rec_context::end_child_context(xmlns_id_t /*ns*/, xml_token_t /*name*/, xml_context_base* /*child*/)
{
}

void xlsx_pivot_cache_rec_context::start_element(xmlns_id_t ns, xml_token_t name, const::std::vector<xml_token_attr_t>& attrs)
{
    xml_token_pair_t parent = push_stack(ns, name);

    if (ns != NS_ooxml_xlsx)
        return;

    switch (name)
    {
        case XML_pivotCacheRecords:
        {
            xml_element_expected(parent, XMLNS_UNKNOWN_ID, XML_UNKNOWN_TOKEN);
            long count = single_long_attr_getter::get(attrs, NS_ooxml_xlsx, XML_count);

            if (get_config().debug)
            {
                cout << "---" << endl;
                cout << "pivot cache record (count: " << count << ")" << endl;
            }

            m_pc_records.set_record_count(count);
            break;
        }
        case XML_r: // record
            xml_element_expected(parent, NS_ooxml_xlsx, XML_pivotCacheRecords);
            if (get_config().debug)
                cout << "* record" << endl;

            break;
        case XML_s: // character value
        {
            xml_element_expected(parent, NS_ooxml_xlsx, XML_r);

            std::string_view cv = single_attr_getter::get(attrs, NS_ooxml_xlsx, XML_v);

            if (get_config().debug)
                cout << "  * s = '" << cv << "'" << endl;

            m_pc_records.append_record_value_character(cv);
            break;
        }
        case XML_x: // shared item index
        {
            xml_element_expected(parent, NS_ooxml_xlsx, XML_r);
            long v = single_long_attr_getter::get(attrs, NS_ooxml_xlsx, XML_v);
            if (get_config().debug)
                cout << "  * x = " << v << endl;

            m_pc_records.append_record_value_shared_item(v);
            break;
        }
        case XML_n: // numeric
        {
            xml_element_expected(parent, NS_ooxml_xlsx, XML_r);
            double val = single_double_attr_getter::get(attrs, NS_ooxml_xlsx, XML_v);
            if (get_config().debug)
                cout << "  * n = " << val << endl;

            m_pc_records.append_record_value_numeric(val);
            break;
        }
        case XML_e: // error value
        {
            pstring cv = single_attr_getter::get(attrs, NS_ooxml_xlsx, XML_v);

            if (get_config().debug)
                cout << "  * e = " << cv << endl;

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

void xlsx_pivot_cache_rec_context::characters(std::string_view /*str*/, bool /*transient*/)
{
}

xlsx_pivot_table_context::xlsx_pivot_table_context(session_context& cxt, const tokens& tokens) :
    xml_context_base(cxt, tokens) {}

xml_context_base* xlsx_pivot_table_context::create_child_context(xmlns_id_t /*ns*/, xml_token_t /*name*/)
{
    return nullptr;
}

void xlsx_pivot_table_context::end_child_context(xmlns_id_t /*ns*/, xml_token_t /*name*/, xml_context_base* /*child*/)
{
}

void xlsx_pivot_table_context::start_element(xmlns_id_t ns, xml_token_t name, const::std::vector<xml_token_attr_t>& attrs)
{
    xml_token_pair_t parent = push_stack(ns, name);
    if (ns == NS_ooxml_xlsx)
    {
        switch (name)
        {
            case XML_pivotTableDefinition:
            {
                xml_element_expected(parent, XMLNS_UNKNOWN_ID, XML_UNKNOWN_TOKEN);
                if (get_config().debug)
                    cout << "---" << endl;
                for (const xml_token_attr_t& attr : attrs)
                {
                    if (attr.ns && attr.ns != NS_ooxml_xlsx)
                        continue;

                    long v = 0;
                    bool b = false;

                    switch (attr.name)
                    {
                        case XML_name:
                            if (get_config().debug)
                                cout << "name: " << attr.value << endl;
                            break;
                        case XML_cacheId:
                            v = to_long(attr.value);
                            if (get_config().debug)
                                cout << "cache ID: " << v << endl;
                            break;
                        case XML_applyNumberFormats:
                            b = to_bool(attr.value);
                            if (get_config().debug)
                                cout << "apply number formats: " << b << endl;
                            break;
                        case XML_applyBorderFormats:
                            b = to_bool(attr.value);
                            if (get_config().debug)
                                cout << "apply border formats: " << b << endl;
                            break;
                        case XML_applyFontFormats:
                            b = to_bool(attr.value);
                            if (get_config().debug)
                                cout << "apply font formats: " << b << endl;
                            break;
                        case XML_applyPatternFormats:
                            b = to_bool(attr.value);
                            if (get_config().debug)
                                cout << "apply pattern formats: " << b << endl;
                            break;
                        case XML_applyAlignmentFormats:
                            b = to_bool(attr.value);
                            if (get_config().debug)
                                cout << "apply alignment formats: " << b << endl;
                            break;
                        case XML_applyWidthHeightFormats:
                            b = to_bool(attr.value);
                            if (get_config().debug)
                                cout << "apply width/height formats: " << b << endl;
                            break;
                        case XML_dataCaption:
                            if (get_config().debug)
                                cout << "data caption: " << attr.value << endl;
                            break;
                        case XML_updatedVersion:
                            v = to_long(attr.value);
                            if (get_config().debug)
                                cout << "updated version: " << v << endl;
                            break;
                        case XML_minRefreshableVersion:
                            v = to_long(attr.value);
                            if (get_config().debug)
                                cout << "minimum refreshable version: " << v << endl;
                            break;
                        case XML_showCalcMbrs:
                            b = to_bool(attr.value);
                            if (get_config().debug)
                                cout << "show calc members (?): " << b << endl;
                            break;
                        case XML_useAutoFormatting:
                            b = to_bool(attr.value);
                            if (get_config().debug)
                                cout << "use auto formatting: " << b << endl;
                            break;
                        case XML_itemPrintTitles:
                            b = to_bool(attr.value);
                            if (get_config().debug)
                                cout << "item print titles (?): " << b << endl;
                            break;
                        case XML_createdVersion:
                            v = to_long(attr.value);
                            if (get_config().debug)
                                cout << "created version: " << v << endl;
                            break;
                        case XML_indent:
                            b = to_bool(attr.value);
                            if (get_config().debug)
                                cout << "indent: " << b << endl;
                            break;
                        case XML_compact:
                            b = to_bool(attr.value);
                            if (get_config().debug)
                                cout << "compact: " << b << endl;
                            break;
                        case XML_compactData:
                            b = to_bool(attr.value);
                            if (get_config().debug)
                                cout << "compact data: " << b << endl;
                            break;
                        case XML_outline:
                            b = to_bool(attr.value);
                            if (get_config().debug)
                                cout << "outline: " << b << endl;
                            break;
                        case XML_outlineData:
                            b = to_bool(attr.value);
                            if (get_config().debug)
                                cout << "outline data: " << b << endl;
                            break;
                        case XML_gridDropZones:
                            b = to_bool(attr.value);
                            if (get_config().debug)
                                cout << "grid drop zones: " << b << endl;
                            break;
                        case XML_multipleFieldFilters:
                            b = to_bool(attr.value);
                            if (get_config().debug)
                                cout << "multiple field filters: " << b << endl;
                            break;
                        default:
                            ;
                    }
                }
            }
            break;
            case XML_location:
            {
                xml_element_expected(parent, NS_ooxml_xlsx, XML_pivotTableDefinition);

                for (const xml_token_attr_t& attr : attrs)
                {
                    if (attr.ns && attr.ns != NS_ooxml_xlsx)
                        continue;

                    long v = -1;
                    switch (attr.name)
                    {
                        case XML_ref:
                            if (get_config().debug)
                                cout << "ref: " << attr.value << endl;
                            break;
                        case XML_firstHeaderRow:
                            v = to_long(attr.value);
                            if (get_config().debug)
                                cout << "first header row: " << v << endl;
                            break;
                        case XML_firstDataRow:
                            v = to_long(attr.value);
                            if (get_config().debug)
                                cout << "first data row: " << v << endl;
                            break;
                        case XML_firstDataCol:
                            v = to_long(attr.value);
                            if (get_config().debug)
                                cout << "first data column: " << v << endl;
                            break;
                        default:
                            ;
                    }
                }
            }
            break;
            case XML_pivotFields:
            {
                // pivotFields and its child elements represent the visual
                // appearances of the fields inside pivot table.
                xml_element_expected(parent, NS_ooxml_xlsx, XML_pivotTableDefinition);
                size_t count = single_long_attr_getter::get(attrs, NS_ooxml_xlsx, XML_count);
                if (get_config().debug)
                    cout << "field count: " << count << endl;
            }
            break;
            case XML_pivotField:
            {
                xml_element_expected(parent, NS_ooxml_xlsx, XML_pivotFields);

                if (get_config().debug)
                    cout << "---" << endl;

                for (const xml_token_attr_t& attr : attrs)
                {
                    if (attr.ns && attr.ns != NS_ooxml_xlsx)
                        continue;

                    switch (attr.name)
                    {
                        case XML_axis:
                            if (get_config().debug)
                                cout << "  * axis: " << attr.value << endl;
                        break;
                        case XML_compact:
                        {
                            bool b = to_bool(attr.value);
                            if (get_config().debug)
                                cout << "  * compact: " << b << endl;
                        }
                        break;
                        case XML_outline:
                        {
                            bool b = to_bool(attr.value);
                            if (get_config().debug)
                                cout << "  * outline: " << b << endl;
                        }
                        break;
                        case XML_showAll:
                        {
                            bool b = to_bool(attr.value);
                            if (get_config().debug)
                                cout << "  * show all: " << b << endl;
                        }
                        break;
                        case XML_dataField:
                        {
                            bool b = to_bool(attr.value);
                            if (get_config().debug)
                                cout << "  * data field: " << b << endl;
                        }
                        break;
                        default:
                            ;
                    }
                }
            }
            break;
            case XML_items:
            {
                xml_element_expected(parent, NS_ooxml_xlsx, XML_pivotField);
                size_t count = single_long_attr_getter::get(attrs, NS_ooxml_xlsx, XML_count);
                if (get_config().debug)
                    cout << "  * item count: " << count << endl;
            }
            break;
            case XML_item:
            {
                xml_element_expected(parent, NS_ooxml_xlsx, XML_items);

                for (const xml_token_attr_t& attr : attrs)
                {
                    if (attr.ns && attr.ns != NS_ooxml_xlsx)
                        continue;

                    switch (attr.name)
                    {
                        case XML_x:
                        {
                            // field item index as defined in the pivot cache.
                            long idx = to_long(attr.value);
                            if (get_config().debug)
                                cout << "    * x = " << idx << endl;
                        }
                        break;
                        case XML_t:
                        {
                            // When the <item> element has attribute 't', it's subtotal or
                            // some sort of function item.  See 3.18.45 ST_ItemType
                            // (PivotItem Type) for possible values.
                            if (get_config().debug)
                                cout << "    * type = " << attr.value << endl;
                        }
                        break;
                        default:
                            ;
                    }
                }
            }
            break;
            case XML_rowFields:
            {
                xml_element_expected(parent, NS_ooxml_xlsx, XML_pivotTableDefinition);
                size_t count = single_long_attr_getter::get(attrs, NS_ooxml_xlsx, XML_count);
                if (get_config().debug)
                {
                    cout << "---" << endl;
                    cout << "row field count: " << count << endl;
                }
            }
            break;
            case XML_colFields:
            {
                xml_element_expected(parent, NS_ooxml_xlsx, XML_pivotTableDefinition);
                size_t count = single_long_attr_getter::get(attrs, NS_ooxml_xlsx, XML_count);
                if (get_config().debug)
                {
                    cout << "---" << endl;
                    cout << "column field count: " << count << endl;
                }
            }
            break;
            case XML_pageFields:
            {
                xml_element_expected(parent, NS_ooxml_xlsx, XML_pivotTableDefinition);
                size_t count = single_long_attr_getter::get(attrs, NS_ooxml_xlsx, XML_count);
                if (get_config().debug)
                {
                    cout << "---" << endl;
                    cout << "page field count: " << count << endl;
                }
            }
            break;
            case XML_pageField:
            {
                xml_element_expected(parent, NS_ooxml_xlsx, XML_pageFields);

                if (get_config().debug)
                    cout << "  * page field:";

                for (const xml_token_attr_t& attr : attrs)
                {
                    if (attr.ns && attr.ns != NS_ooxml_xlsx)
                        continue;

                    switch (attr.name)
                    {
                        case XML_fld:
                        {
                            long fld = to_long(attr.value);
                            if (get_config().debug)
                                cout << "field index = " << fld << "; ";
                            break;
                        }
                        case XML_item:
                        {
                            long item = to_long(attr.value);
                            if (get_config().debug)
                                cout << "item index = " << item << "; ";
                            break;
                        }
                        case XML_hier:
                        {
                            long hier = to_long(attr.value);
                            // -1 if not applicable.
                            if (get_config().debug)
                                cout << "OLAP hierarchy index = " << hier << "; ";
                            break;
                        }
                        default:
                            ;
                    }
                }

                if (get_config().debug)
                    cout << endl;
                break;
            }
            case XML_field:
            {
                xml_elem_stack_t expected;
                expected.reserve(3);
                expected.push_back(xml_token_pair_t(NS_ooxml_xlsx, XML_rowFields));
                expected.push_back(xml_token_pair_t(NS_ooxml_xlsx, XML_colFields));
                xml_element_expected(parent, expected);

                // Index into the list of <pivotField> collection which is
                // given earlier under the <pivotFields> element.  The value
                // of -2 represents a special field that displays the list of
                // data fields when the pivot table contains more than one
                // data field.
                long idx = single_long_attr_getter::get(attrs, NS_ooxml_xlsx, XML_x);
                if (get_config().debug)
                    cout << "  * x = " << idx << endl;
            }
            break;
            case XML_dataFields:
            {
                xml_element_expected(parent, NS_ooxml_xlsx, XML_pivotTableDefinition);
                size_t count = single_long_attr_getter::get(attrs, NS_ooxml_xlsx, XML_count);
                if (get_config().debug)
                {
                    cout << "---" << endl;
                    cout << "data field count: " << count << endl;
                }
            }
            break;
            case XML_dataField:
            {
                xml_element_expected(parent, NS_ooxml_xlsx, XML_dataFields);

                if (get_config().debug)
                    cout << "  * data field: ";

                for (const xml_token_attr_t& attr : attrs)
                {
                    if (attr.ns && attr.ns != NS_ooxml_xlsx)
                        continue;

                    switch (attr.name)
                    {
                        case XML_name:
                        {
                            if (get_config().debug)
                                cout << "name = " << attr.value << "; ";
                            break;
                        }
                        case XML_fld:
                        {
                            long fld = to_long(attr.value);
                            if (get_config().debug)
                                cout << "field = " << fld << "; ";
                            break;
                        }
                        case XML_baseField:
                        {
                            long fld = to_long(attr.value);
                            if (get_config().debug)
                                cout << "base field = " << fld << "; ";
                            break;
                        }
                        case XML_baseItem:
                        {
                            long fld = to_long(attr.value);
                            if (get_config().debug)
                                cout << "base item = " << fld << "; ";
                            break;
                        }
                        case XML_subtotal:
                        {
                            if (get_config().debug)
                                cout << "subtotal = " << attr.value << "; ";
                            break;
                        }
                        default:
                            ;
                    }
                }

                if (get_config().debug)
                    cout << endl;
            }
            break;
            case XML_rowItems:
            {
                // <rowItems> structure describes the displayed content of
                // cells in the row field area.  Each <i> child element
                // represents a single row.
                xml_element_expected(parent, NS_ooxml_xlsx, XML_pivotTableDefinition);
                size_t count = single_long_attr_getter::get(attrs, NS_ooxml_xlsx, XML_count);
                if (get_config().debug)
                {
                    cout << "---" << endl;
                    cout << "row item count: " << count << endl;
                }
            }
            break;
            case XML_colItems:
            {
                xml_element_expected(parent, NS_ooxml_xlsx, XML_pivotTableDefinition);
                size_t count = single_long_attr_getter::get(attrs, NS_ooxml_xlsx, XML_count);
                if (get_config().debug)
                {
                    cout << "---" << endl;
                    cout << "column item count: " << count << endl;
                }
            }
            break;
            case XML_i:
            {
                xml_elem_stack_t expected;
                expected.reserve(2);
                expected.push_back(xml_token_pair_t(NS_ooxml_xlsx, XML_rowItems));
                expected.push_back(xml_token_pair_t(NS_ooxml_xlsx, XML_colItems));
                xml_element_expected(parent, expected);

                if (get_config().debug)
                    cout << "---" << endl;

                for (const xml_token_attr_t& attr : attrs)
                {
                    if (attr.ns && attr.ns != NS_ooxml_xlsx)
                        continue;

                    switch (attr.name)
                    {
                        case XML_t:
                        {
                            // total or subtotal function type.
                            if (get_config().debug)
                                cout << "  * type = " << attr.value << endl;
                        }
                        break;
                        case XML_r:
                        {
                            // "repeated item count" which basically is the number of
                            // blank cells that occur after the preivous non-empty cell on
                            // the same row (in the classic layout mode).
                            long v = to_long(attr.value);
                            if (get_config().debug)
                                cout << "  * repeat item count = " << v << endl;
                        }
                        break;
                        case XML_i:
                        {
                            // zero-based data field index in case of multiple data fields.
                            long v = to_long(attr.value);
                            if (get_config().debug)
                                cout << "  * data field index = " << v << endl;
                        }
                        break;
                        default:
                            ;
                    }
                }
            }
            break;
            case XML_x:
            {
                if (parent.first != NS_ooxml_xlsx)
                {
                    warn_unhandled();
                    break;
                }

                if (parent.second == XML_i)
                {
                    long idx = single_long_attr_getter::get(attrs, NS_ooxml_xlsx, XML_v);
                    if (idx < 0)
                        // 0 is default when not set.
                        idx = 0;

                    if (get_config().debug)
                        cout << "  * v = " << idx << endl;
                    break;
                }

                warn_unhandled();
            }
            break;
            case XML_pivotTableStyleInfo:
            {
                xml_element_expected(parent, NS_ooxml_xlsx, XML_pivotTableDefinition);

                if (get_config().debug)
                {
                    cout << "---" << endl;
                    cout << "* style info: ";
                }

                for (const xml_token_attr_t& attr : attrs)
                {
                    if (attr.ns && attr.ns != NS_ooxml_xlsx)
                        continue;

                    bool b = false;

                    switch (attr.name)
                    {
                        case XML_name:
                            if (get_config().debug)
                                cout << "name='" << attr.value << "'; ";
                            break;
                        case XML_showRowHeaders:
                            b = to_bool(attr.value);
                            if (get_config().debug)
                                cout << "show row headers=" << b << "; ";
                            break;
                        case XML_showColHeaders:
                            b = to_bool(attr.value);
                            if (get_config().debug)
                                cout << "show column headers=" << b << "; ";
                        break;
                        case XML_showRowStripes:
                            b = to_bool(attr.value);
                            if (get_config().debug)
                                cout << "show row stripes=" << b << "; ";
                        break;
                        case XML_showColStripes:
                            b = to_bool(attr.value);
                            if (get_config().debug)
                                cout << "show column stripes=" << b << "; ";
                        break;
                        case XML_showLastColumn:
                            b = to_bool(attr.value);
                            if (get_config().debug)
                                cout << "show last column=" << b << "; ";
                        break;
                        default:
                            ;
                    }
                }

                if (get_config().debug)
                    cout << endl;
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
    return pop_stack(ns, name);
}

void xlsx_pivot_table_context::characters(std::string_view /*str*/, bool /*transient*/)
{
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
