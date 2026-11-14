/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "xlsx_conditional_format_context.hpp"
#include "xlsx_helper.hpp"
#include "xlsx_types.hpp"
#include "ooxml_token_constants.hpp"
#include "ooxml_namespace_types.hpp"
#include "impl_utils.hpp"
#include "string_helper.hpp"

#include "orcus/exception.hpp"
#include "orcus/spreadsheet/import_interface.hpp"
#include "orcus/spreadsheet/import_interface_cond_format.hpp"
#include "orcus/measurement.hpp"

#include <mdds/sorted_string_map.hpp>

#include <optional>

namespace ss = orcus::spreadsheet;

namespace orcus {

namespace {

enum xlsx_cond_format_type
{
    none = 0,
    expression,
    cellIs,
    colorScale,
    dataBar,
    iconSet,
    top10,
    uniqueValues,
    duplicateValues,
    containsText,
    notContainsText,
    beginsWith,
    endsWith,
    containsBlanks,
    notContainsBlanks,
    containsErrors,
    notContainsErrors,
    timePeriod,
    aboveAverage
};

enum xlsx_cond_format_operator
{
    operator_default = 0,
    operator_beginsWith,
    operator_between,
    operator_containsText,
    operator_endsWith,
    operator_equal,
    operator_greaterThan,
    operator_greaterThanOrEqual,
    operator_lessThan,
    operator_lessThanOrEqual,
    operator_notBetween,
    operator_notContains,
    operator_notEqual
};

enum xlsx_cond_format_date
{
    date_default = 0,
    date_last7Days,
    date_lastMonth,
    date_lastWeek,
    date_nextMonth,
    date_thisMonth,
    date_thisWeek,
    date_today,
    date_tomorrow,
    date_yesterday
};

namespace cond_type {

using map_type = mdds::sorted_string_map<xlsx_cond_format_type>;

// keys must be sorted
constexpr map_type::entry_type entries[] =
{
    { "aboveAverage", aboveAverage },
    { "beginsWith", beginsWith },
    { "cellIs", cellIs },
    { "colorScale", colorScale },
    { "containsBlanks", containsBlanks },
    { "containsErrors", containsErrors },
    { "containsText", containsText },
    { "dataBar", dataBar },
    { "duplicateValues", duplicateValues },
    { "endsWith", endsWith },
    { "expression", expression },
    { "iconSet", iconSet },
    { "notContainsErrors", notContainsErrors },
    { "notContainsText", notContainsText },
    { "timePeriod", timePeriod },
    { "top10", top10 },
    { "uniqueValues", uniqueValues }
};

const map_type& get()
{
    static const map_type mt(entries, none);
    return mt;
}

} // namespace cond_type

namespace cond_operator {

using map_type = mdds::sorted_string_map<xlsx_cond_format_operator>;

// keys must be sorted
constexpr map_type::entry_type entries[] =
{
    { "beginsWith", operator_beginsWith },
    { "between", operator_between },
    { "containsText", operator_containsText },
    { "endsWith", operator_endsWith },
    { "equal", operator_equal },
    { "greaterThan", operator_greaterThan },
    { "greaterThanOrEqual", operator_greaterThanOrEqual },
    { "lessThan", operator_lessThan },
    { "lessThanOrEqual", operator_lessThanOrEqual },
    { "notBetween", operator_notBetween },
    { "notContains", operator_notContains },
    { "notEqual", operator_notEqual }
};

const map_type& get()
{
    static const map_type mt(entries, operator_default);
    return mt;
}

} // namespace cond_operator

namespace cond_date {

using map_type = mdds::sorted_string_map<xlsx_cond_format_date>;

// keys must be sorted
constexpr map_type::entry_type entries[] =
{
    { "last7Days", date_last7Days },
    { "lastMonth", date_lastMonth },
    { "lastWeek", date_lastWeek },
    { "nextMonth", date_nextMonth },
    { "thisMonth", date_thisMonth },
    { "thisWeek", date_thisWeek },
    { "today", date_today },
    { "tomorrow", date_tomorrow },
    { "yesterday", date_yesterday }
};

const map_type& get()
{
    static const map_type mt(entries, date_default);
    return mt;
}

} // namespace cond_date

struct cf_rule_attrs
{
    xlsx_cond_format_type type = none;
    xlsx_cond_format_operator op = operator_default;
    xlsx_cond_format_date date = date_default;
    std::optional<std::size_t> dxf_id;
    bool above_average = true;
    bool equal_average = false;
    bool percent = false;
    bool bottom = false;
    std::string_view text;
    std::string_view std_dev;
    std::string_view rank;
};

ss::cond_format_operator_t to_cell_is_operator(xlsx_cond_format_operator op)
{
    switch (op)
    {
        case operator_beginsWith:
            return ss::cond_format_operator_t::begins_with;
        case operator_between:
            return ss::cond_format_operator_t::between;
        case operator_containsText:
            return ss::cond_format_operator_t::contains;
        case operator_endsWith:
            return ss::cond_format_operator_t::ends_with;
        case operator_equal:
            return ss::cond_format_operator_t::equal;
        case operator_greaterThan:
            return ss::cond_format_operator_t::greater;
        case operator_greaterThanOrEqual:
            return ss::cond_format_operator_t::greater_equal;
        case operator_lessThan:
            return ss::cond_format_operator_t::less;
        case operator_lessThanOrEqual:
            return ss::cond_format_operator_t::less_equal;
        case operator_notBetween:
            return ss::cond_format_operator_t::not_between;
        case operator_notContains:
            return ss::cond_format_operator_t::not_contains;
        case operator_notEqual:
            return ss::cond_format_operator_t::not_equal;
        default:
            return ss::cond_format_operator_t::expression;
    }
}

ss::cond_format_operator_t to_operator(const cf_rule_attrs& rule)
{
    switch (rule.type)
    {
        case expression:
            return ss::cond_format_operator_t::expression;
        case cellIs:
            return to_cell_is_operator(rule.op);
        case top10:
            if (rule.percent)
                return rule.bottom ? ss::cond_format_operator_t::bottom_percent : ss::cond_format_operator_t::top_percent;

            return rule.bottom ? ss::cond_format_operator_t::bottom_n : ss::cond_format_operator_t::top_n;
        case uniqueValues:
            return ss::cond_format_operator_t::unique;
        case duplicateValues:
            return ss::cond_format_operator_t::duplicate;
        case containsText:
            return ss::cond_format_operator_t::contains;
        case notContainsText:
            return ss::cond_format_operator_t::not_contains;
        case beginsWith:
            return ss::cond_format_operator_t::begins_with;
        case endsWith:
            return ss::cond_format_operator_t::ends_with;
        case containsBlanks:
            return ss::cond_format_operator_t::contains_blanks;
        case containsErrors:
            return ss::cond_format_operator_t::contains_error;
        case notContainsErrors:
            return ss::cond_format_operator_t::contains_no_error;
        case aboveAverage:
            if (rule.above_average)
                return rule.equal_average ? ss::cond_format_operator_t::above_equal_average : ss::cond_format_operator_t::above_average;
            return rule.equal_average ? ss::cond_format_operator_t::below_equal_average : ss::cond_format_operator_t::below_average;
        default:
            return ss::cond_format_operator_t::unknown;
    }
}

ss::cond_format_date_t to_period(xlsx_cond_format_date date)
{
    switch (date)
    {
        case date_last7Days:
            return ss::cond_format_date_t::last_7_days;
        case date_lastMonth:
            return ss::cond_format_date_t::last_month;
        case date_lastWeek:
            return ss::cond_format_date_t::last_week;
        case date_nextMonth:
            return ss::cond_format_date_t::next_month;
        case date_thisMonth:
            return ss::cond_format_date_t::this_month;
        case date_thisWeek:
            return ss::cond_format_date_t::this_week;
        case date_today:
            return ss::cond_format_date_t::today;
        case date_tomorrow:
            return ss::cond_format_date_t::tomorrow;
        case date_yesterday:
            return ss::cond_format_date_t::yesterday;
        default:
            return ss::cond_format_date_t::unknown;
    }
}

enum xlsx_cond_format_cfvo_type
{
    cfvo_default = 0,
    cfvo_num,
    cfvo_percent,
    cfvo_max,
    cfvo_min,
    cfvo_formula,
    cfvo_percentile
};

namespace cfvo_type {

using map_type = mdds::sorted_string_map<xlsx_cond_format_cfvo_type>;

// keys must be sorted
constexpr map_type::entry_type entries[] =
{
    { "formula", cfvo_formula },
    { "max", cfvo_max },
    { "min", cfvo_min },
    { "num", cfvo_num },
    { "percent", cfvo_percent },
    { "percentile", cfvo_percentile },
};

const map_type& get()
{
    static const map_type mt(entries, cfvo_default);
    return mt;
}

} // namespace cfvo_type

namespace icon_set {

using map_type = mdds::sorted_string_map<ss::icon_set_t>;

// keys must be sorted
constexpr map_type::entry_type entries[] = {
    { "3Arrows",         ss::icon_set_t::three_arrows           },
    { "3ArrowsGray",     ss::icon_set_t::three_arrows_gray      },
    { "3Flags",          ss::icon_set_t::three_flags            },
    { "3Signs",          ss::icon_set_t::three_signs            },
    { "3Stars",          ss::icon_set_t::three_stars            },
    { "3Symbols",        ss::icon_set_t::three_symbols          },
    { "3Symbols2",       ss::icon_set_t::three_symbols_2        },
    { "3TrafficLights1", ss::icon_set_t::three_traffic_lights_1 },
    { "3TrafficLights2", ss::icon_set_t::three_traffic_lights_2 },
    { "3Triangles",      ss::icon_set_t::three_triangles        },
    { "4Arrows",         ss::icon_set_t::four_arrows            },
    { "4ArrowsGray",     ss::icon_set_t::four_arrows_gray       },
    { "4Rating",         ss::icon_set_t::four_rating            },
    { "4RedToBlack",     ss::icon_set_t::four_red_to_black      },
    { "4TrafficLights",  ss::icon_set_t::four_traffic_lights    },
    { "5Arrows",         ss::icon_set_t::five_arrows            },
    { "5ArrowsGray",     ss::icon_set_t::five_arrows_gray       },
    { "5Boxes",          ss::icon_set_t::five_boxes             },
    { "5Quarters",       ss::icon_set_t::five_quarters          },
    { "5Rating",         ss::icon_set_t::five_rating            },
    { "NoIcons",         ss::icon_set_t::no_icons               },
};

const map_type& get()
{
    static const map_type mt(entries, ss::icon_set_t::unknown);
    return mt;
}

} // namespace icon_set

}

struct cfvo_values
{
    cfvo_values():
        m_include_equal(true),
        m_type(cfvo_default)
    {
    }

    bool m_include_equal;
    xlsx_cond_format_cfvo_type m_type;
    std::string_view m_value;
};

xlsx_conditional_format_context::xlsx_conditional_format_context(
        session_context& session_cxt, const tokens& tokens,
        ss::iface::import_reference_resolver& resolver,
        ss::iface::import_sheet& sheet):
    xml_context_base(session_cxt, tokens),
    m_resolver(resolver),
    m_sheet(sheet)
{
    static const xml_element_validator::rule rules[] = {
        // parent element -> child element
        { XMLNS_UNKNOWN_ID, XML_UNKNOWN_TOKEN, NS_ooxml_xlsx, XML_conditionalFormatting }, // root element
        { NS_ooxml_xlsx, XML_conditionalFormatting, NS_ooxml_xlsx, XML_cfRule },
        { NS_ooxml_xlsx, XML_cfRule, NS_ooxml_xlsx, XML_formula },
        { NS_ooxml_xlsx, XML_cfRule, NS_ooxml_xlsx, XML_colorScale },
        { NS_ooxml_xlsx, XML_cfRule, NS_ooxml_xlsx, XML_dataBar },
        { NS_ooxml_xlsx, XML_cfRule, NS_ooxml_xlsx, XML_iconSet },
        { NS_ooxml_xlsx, XML_cfRule, NS_ooxml_xlsx, XML_extLst },
        { NS_ooxml_xlsx, XML_colorScale, NS_ooxml_xlsx, XML_cfvo },
        { NS_ooxml_xlsx, XML_colorScale, NS_ooxml_xlsx, XML_color },
        { NS_ooxml_xlsx, XML_dataBar, NS_ooxml_xlsx, XML_cfvo },
        { NS_ooxml_xlsx, XML_dataBar, NS_ooxml_xlsx, XML_color },
        { NS_ooxml_xlsx, XML_iconSet, NS_ooxml_xlsx, XML_cfvo },
        { NS_ooxml_xlsx, XML_extLst, NS_ooxml_xlsx, XML_ext },
        { NS_ooxml_xlsx, XML_ext, NS_mso_x14, XML_id },
    };

    init_element_validator(rules, std::size(rules));
}

xlsx_conditional_format_context::~xlsx_conditional_format_context()
{
}

xml_context_base* xlsx_conditional_format_context::create_child_context(xmlns_id_t /*ns*/, xml_token_t /*name*/)
{
    return nullptr;
}

void xlsx_conditional_format_context::end_child_context(xmlns_id_t /*ns*/, xml_token_t /*name*/, xml_context_base* /*child*/)
{
}

void xlsx_conditional_format_context::start_element(xmlns_id_t ns, xml_token_t name, const xml_token_attrs_t& attrs)
{
    push_stack(ns, name);

    switch (name)
    {
        case XML_conditionalFormatting:
            start_conditional_formatting(attrs);
            break;
        case XML_cfRule:
            start_cf_rule(attrs);
            break;
        case XML_cfvo:
            start_cfvo(attrs);
            break;
        case XML_dataBar:
            start_data_bar(attrs);
            break;
        case XML_iconSet:
            start_icon_set(attrs);
            break;
        case XML_color:
        {
            for (const xml_token_attr_t& attr : attrs)
            {
                switch (attr.name)
                {
                    case XML_rgb:
                    {
                        argb_color color;
                        if (to_rgb(attr.value, color.alpha, color.red, color.green, color.blue))
                            m_colors.push_back(color);
                        break;
                    }
                    default:
                        ;
                }
            }
            break;
        }
        case XML_formula:
            break;
        case XML_colorScale:
            break;
        default:
            warn_unhandled();
    }
}

namespace {

void import_cfvo(const cfvo_values& values, ss::iface::import_cond_format_value& dest)
{
    if (!values.m_value.empty())
        dest.set_formula(values.m_value);

    switch (values.m_type)
    {
        case cfvo_num:
            dest.set_type(ss::cond_format_value_type_t::value);
            break;
        case cfvo_percent:
            dest.set_type(ss::cond_format_value_type_t::percent);
            break;
        case cfvo_max:
            dest.set_type(ss::cond_format_value_type_t::max);
            break;
        case cfvo_min:
            dest.set_type(ss::cond_format_value_type_t::min);
            break;
        case cfvo_formula:
            dest.set_type(ss::cond_format_value_type_t::formula);
            break;
        case cfvo_percentile:
            dest.set_type(ss::cond_format_value_type_t::percentile);
            break;
        default:;
    }
}

}

bool xlsx_conditional_format_context::end_element(xmlns_id_t ns, xml_token_t name)
{
    switch (name)
    {
        case XML_conditionalFormatting:
        {
            if (mp_cond_format)
                mp_cond_format->commit();

            mp_cond_format = nullptr;
            break;
        }
        case XML_cfRule:
        {
            commit_rule();
            m_cfvo_values.clear();
            m_colors.clear();
            break;
        }
        case XML_cfvo:
            break;
        case XML_color:
            break;
        case XML_formula:
        {
            if (mp_condition)
            {
                auto* value = mp_condition->start_value();
                ENSURE_INTERFACE(value, import_cond_format_value);

                value->set_formula(m_cur_str);
                value->commit();
            }
            break;
        }
        case XML_dataBar:
        {
            if (m_colors.size() != 1)
                throw general_error("invalid dataBar record");

            if (m_cfvo_values.size() != 2)
                throw general_error("invalid dataBar record");

            if (mp_databar)
            {
                const argb_color& color = m_colors[0];
                mp_databar->set_color_positive(color.alpha, color.red, color.green, color.blue);

                for (const auto& cfvo : m_cfvo_values)
                {
                    auto* value = mp_databar->start_value();
                    ENSURE_INTERFACE(value, import_cond_format_value);

                    import_cfvo(cfvo, *value);
                    value->commit();
                }
            }
            break;
        }
        case XML_iconSet:
        {
            if (m_cfvo_values.size() < 2)
                throw general_error("invalid iconSet record");

            if (mp_iconset)
            {
                for (const auto& cfvo : m_cfvo_values)
                {
                    auto* value = mp_iconset->start_value();
                    ENSURE_INTERFACE(value, import_cond_format_value);

                    import_cfvo(cfvo, *value);
                    value->commit();
                }
            }
            break;
        }
        case XML_colorScale:
        {
            if (m_cfvo_values.size() < 2)
                throw general_error("invalid colorScale record");

            if (m_cfvo_values.size() != m_colors.size())
                throw general_error("invalid colorScale record");

            if (mp_colorscale)
            {
                for (auto it_color = m_colors.begin(); const auto& cfvo : m_cfvo_values)
                {
                    auto* value = mp_colorscale->start_value();
                    ENSURE_INTERFACE(value, import_cond_format_value);

                    import_cfvo(cfvo, *value);
                    value->set_color(it_color->alpha, it_color->red, it_color->green, it_color->blue);
                    value->commit();
                    ++it_color;
                }
            }
            break;
        }
    }

    m_cur_str = std::string_view{};
    return pop_stack(ns, name);
}

void xlsx_conditional_format_context::characters(std::string_view str, bool transient)
{
    m_cur_str = str;
    if (transient)
        m_cur_str = m_pool.intern(m_cur_str).first;
}

void xlsx_conditional_format_context::start_conditional_formatting(const xml_token_attrs_t& attrs)
{
    mp_cond_format = m_sheet.start_cond_format();
    if (!mp_cond_format)
        return;

    for (const xml_token_attr_t& attr : attrs)
    {
        switch (attr.name)
        {
            case XML_sqref:
            {
                // sqref may contain multiple ranges separated by spaces
                for (std::string_view token : string_helper::split_string(attr.value, ' '))
                {
                    if (token.empty())
                        continue;

                    auto range = to_rc_range(m_resolver.resolve_range(token));
                    mp_cond_format->append_range(range);
                }
                break;
            }
        }
    }
}

void xlsx_conditional_format_context::start_cf_rule(const xml_token_attrs_t& attrs)
{
    if (!mp_cond_format)
        return;

    // NB: The attribute values are passed on before this call returns, so there
    // is no need to intern the transient ones.
    cf_rule_attrs rule;

    for (const xml_token_attr_t& attr : attrs)
    {
        switch (attr.name)
        {
            case XML_type:
                rule.type = cond_type::get().find(attr.value);
                break;
            case XML_dxfId:
                rule.dxf_id = to_long(attr.value);
                break;
            case XML_aboveAverage:
                rule.above_average = to_bool(attr.value);
                break;
            case XML_percent:
                rule.percent = to_bool(attr.value);
                break;
            case XML_bottom:
                rule.bottom = to_bool(attr.value);
                break;
            case XML_operator:
                rule.op = cond_operator::get().find(attr.value);
                break;
            case XML_text:
                rule.text = attr.value;
                break;
            case XML_timePeriod:
                rule.date = cond_date::get().find(attr.value);
                break;
            case XML_rank:
                rule.rank = attr.value;
                break;
            case XML_stdDev:
                rule.std_dev = attr.value;
                break;
            case XML_equalAverage:
                rule.equal_average = to_bool(attr.value);
                break;
            default:
                ;
        }
    }

    switch (rule.type)
    {
        case timePeriod:
        {
            mp_date = mp_cond_format->start_date();
            ENSURE_INTERFACE(mp_date, import_cond_format_date);

            mp_date->set_period(to_period(rule.date));

            if (rule.dxf_id)
                mp_date->set_dxf_id(*rule.dxf_id);
            break;
        }
        case colorScale:
        {
            mp_colorscale = mp_cond_format->start_colorscale();
            ENSURE_INTERFACE(mp_colorscale, import_cond_format_colorscale);
            break;
        }
        case dataBar:
        {
            mp_databar = mp_cond_format->start_databar();
            ENSURE_INTERFACE(mp_databar, import_cond_format_databar);
            break;
        }
        case iconSet:
        {
            mp_iconset = mp_cond_format->start_iconset();
            ENSURE_INTERFACE(mp_iconset, import_cond_format_iconset);
            break;
        }
        case none:
            warn("unknown conditional format rule type");
            break;
        default:
        {
            mp_condition = mp_cond_format->start_condition();
            ENSURE_INTERFACE(mp_condition, import_cond_format_condition);

            mp_condition->set_operator(to_operator(rule));

            if (rule.dxf_id)
                mp_condition->set_dxf_id(*rule.dxf_id);

            if (!rule.rank.empty())
                mp_condition->set_rank(rule.rank);

            if (!rule.text.empty())
                mp_condition->set_text(rule.text);

            if (!rule.std_dev.empty())
                mp_condition->set_std_dev(rule.std_dev);
        }
    }
}

void xlsx_conditional_format_context::start_cfvo(const xml_token_attrs_t& attrs)
{
    cfvo_values values;

    for (const xml_token_attr_t& attr : attrs)
    {
        switch (attr.name)
        {
            case XML_gte:
                values.m_include_equal = to_bool(attr.value);
                break;
            case XML_type:
                values.m_type = cfvo_type::get().find(attr.value);
                break;
            case XML_val:
                if (attr.transient)
                {
                    values.m_value = m_pool.intern(attr.value).first;
                }
                else
                {
                    values.m_value = attr.value;
                }
                break;
            default:;
        }
    }

    m_cfvo_values.push_back(values);
}

void xlsx_conditional_format_context::start_data_bar(const xml_token_attrs_t& attrs)
{
    bool show_value = true;
    std::size_t min_length = 10;
    std::size_t max_length = 90;

    for (const xml_token_attr_t& attr : attrs)
    {
        switch (attr.name)
        {
            case XML_showValue:
                show_value = to_bool(attr.value);
                break;
            case XML_maxLength:
                max_length = to_long(attr.value);
                break;
            case XML_minLength:
                min_length = to_long(attr.value);
                break;
            default:;
        }
    }

    if (!mp_databar)
        return;

    mp_databar->set_show_value(show_value);
    mp_databar->set_min_length(min_length);
    mp_databar->set_max_length(max_length);
}

void xlsx_conditional_format_context::start_icon_set(const xml_token_attrs_t& attrs)
{
    bool reverse = false;
    bool show_value = true;
    ss::icon_set_t icon_set_type = ss::icon_set_t::three_arrows;

    for (const xml_token_attr_t& attr : attrs)
    {
        switch (attr.name)
        {
            case XML_iconSet:
                icon_set_type = icon_set::get().find(attr.value);
                break;
            case XML_reverse:
                reverse = to_bool(attr.value);
                break;
            case XML_showValue:
                show_value = to_bool(attr.value);
                break;
            default:;
        }
    }

    if (!mp_iconset)
        return;

    mp_iconset->set_show_value(show_value);
    mp_iconset->set_reverse(reverse);
    mp_iconset->set_icon_set(icon_set_type);
}

void xlsx_conditional_format_context::commit_rule()
{
    if (mp_condition)
        mp_condition->commit();
    else if (mp_date)
        mp_date->commit();
    else if (mp_colorscale)
        mp_colorscale->commit();
    else if (mp_databar)
        mp_databar->commit();
    else if (mp_iconset)
        mp_iconset->commit();

    mp_condition = nullptr;
    mp_date = nullptr;
    mp_colorscale = nullptr;
    mp_databar = nullptr;
    mp_iconset = nullptr;
}

void xlsx_conditional_format_context::reset()
{
    mp_cond_format = nullptr;
    commit_rule();

    m_pool.clear();
    m_cur_str = std::string_view{};
    m_cfvo_values.clear();
    m_colors.clear();
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
