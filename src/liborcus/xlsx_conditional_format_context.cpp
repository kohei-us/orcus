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

} // anonymous namespace

struct cfRule_attr_parser
{
    cfRule_attr_parser():
        m_type(none),
        m_operator(operator_default),
        m_date(date_default),
        m_above_average(true),
        m_equal_average(false),
        m_percent(false),
        m_bottom(false)
    {
    }

    void operator()(const xml_token_attr_t& attr)
    {
        switch(attr.name)
        {
            case XML_type:
                m_type = cond_type::get().find(attr.value);
            break;
            case XML_dxfId:
                m_dxf_id = to_long(attr.value);
            break;
            case XML_aboveAverage:
            {
                m_above_average = to_bool(attr.value);
            }
            break;
            case XML_percent:
                m_percent = to_bool(attr.value);
            break;
            case XML_bottom:
                m_bottom = to_bool(attr.value);
            break;
            case XML_operator:
                m_operator = cond_operator::get().find(attr.value);
            break;
            case XML_text:
                // do we need to worry about the transient flag here?
                m_text = attr.value;
            break;
            case XML_timePeriod:
                m_date = cond_date::get().find(attr.value);
            break;
            case XML_rank:
                // do we need to worry about the transient flag here?
                m_rank = attr.value;
            break;
            case XML_stdDev:
                // do we need to worry about the transient flag here?
                m_std_dev = attr.value;
            break;
            case XML_equalAverage:
                m_equal_average = to_bool(attr.value);
            break;
            default:
                break;
        }
    }

    xlsx_cond_format_type get_type() const { return m_type; }

    ss::cond_format_operator_t get_operator() const
    {
        switch (m_type)
        {
            case expression:
                return ss::cond_format_operator_t::expression;
            case cellIs:
                switch (m_operator)
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
            case top10:
                return m_bottom ? ss::cond_format_operator_t::bottom_n : ss::cond_format_operator_t::top_n;
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
                if (m_above_average)
                    return m_equal_average ? ss::cond_format_operator_t::above_equal_average : ss::cond_format_operator_t::above_average;
                return m_equal_average ? ss::cond_format_operator_t::below_equal_average : ss::cond_format_operator_t::below_average;
            default:
                return ss::cond_format_operator_t::unknown;
        }
    }

    ss::cond_format_date_t get_period() const
    {
        switch (m_date)
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

    const std::optional<std::size_t>& get_dxf_id() const { return m_dxf_id; }
    std::string_view get_text() const { return m_text; }
    std::string_view get_std_dev() const { return m_std_dev; }
    std::string_view get_rank() const { return m_rank; }

private:
    xlsx_cond_format_type m_type;
    xlsx_cond_format_operator m_operator;
    xlsx_cond_format_date m_date;
    std::optional<std::size_t> m_dxf_id;
    bool m_above_average;
    bool m_equal_average;
    bool m_percent;
    bool m_bottom;
    std::string_view m_text;
    std::string_view m_std_dev;
    std::string_view m_rank;
};

namespace {

struct conditional_formatting_attr_parser
{
    conditional_formatting_attr_parser(
        ss::iface::import_cond_format& cond_format, ss::iface::import_reference_resolver& resolver):
        m_cond_format(cond_format),
        m_resolver(resolver)
    {
    }

    void operator()(const xml_token_attr_t& attr)
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
                    m_cond_format.append_range(range);
                }
                break;
            }
        }
    }

private:
    ss::iface::import_cond_format& m_cond_format;
    ss::iface::import_reference_resolver& m_resolver;
};

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

namespace {

struct cfvo_attr_parser
{
    cfvo_attr_parser(string_pool& pool):
        m_string_pool(pool)
    {
    }

    void operator()(const xml_token_attr_t& attr)
    {
        switch (attr.name)
        {
            case XML_gte:
                m_values.m_include_equal = to_bool(attr.value);
            break;
            case XML_type:
                m_values.m_type = cfvo_type::get().find(attr.value);
            break;
            case XML_val:
                if (attr.transient)
                {
                    m_values.m_value = m_string_pool.intern(attr.value).first;
                }
                else
                {
                    m_values.m_value = attr.value;
                }
            break;
            default:
            break;
        }
    }

    cfvo_values get_values()
    {
        return m_values;
    }

private:
    cfvo_values m_values;
    string_pool& m_string_pool;
};

struct data_bar_attr_parser
{
    data_bar_attr_parser():
        m_show_value(true),
        m_min_length(10),
        m_max_length(90)
    {
    }

    void operator()(const xml_token_attr_t& attr)
    {
        switch (attr.name)
        {
            case XML_showValue:
                m_show_value = to_bool(attr.value);
            break;
            case XML_maxLength:
                m_max_length = to_long(attr.value);
            break;
            case XML_minLength:
                m_min_length = to_long(attr.value);
            break;
            default:
            break;
        }
    }

    void import_data(ss::iface::import_cond_format_databar& rule)
    {
        rule.set_show_value(m_show_value);
        rule.set_min_length(m_min_length);
        rule.set_max_length(m_max_length);
    }

private:
    bool m_show_value;
    size_t m_min_length;
    size_t m_max_length;
};

struct icon_set_attr_parser
{
    icon_set_attr_parser():
        m_reverse(false),
        m_percent(true),
        m_show_value(true),
        m_icon_set(ss::icon_set_t::three_arrows)
    {
    }

    void operator()(const xml_token_attr_t& attr)
    {
        switch (attr.name)
        {
            case XML_iconSet:
                m_icon_set = icon_set::get().find(attr.value);
            break;
            case XML_percent:
                m_percent = to_bool(attr.value);
            break;
            case XML_reverse:
                m_reverse = to_bool(attr.value);
            break;
            case XML_showValue:
                m_show_value = to_bool(attr.value);
            break;
            default:
            break;
        }
    }

    void import_data(ss::iface::import_cond_format_iconset& rule)
    {
        rule.set_show_value(m_show_value);
        rule.set_reverse(m_reverse);
        rule.set_icon_set(m_icon_set);
    }

private:
    bool m_reverse;
    bool m_percent;
    bool m_show_value;
    ss::icon_set_t m_icon_set;
};

}

xlsx_conditional_format_context::xlsx_conditional_format_context(
        session_context& session_cxt, const tokens& tokens,
        ss::iface::import_reference_resolver& resolver,
        ss::iface::import_sheet& sheet):
    xml_context_base(session_cxt, tokens),
    m_resolver(resolver),
    m_sheet(sheet)
{
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
    xml_token_pair_t parent = push_stack(ns, name);

    switch (name)
    {
        case XML_conditionalFormatting:
        {
            xml_element_expected(parent, XMLNS_UNKNOWN_ID, XML_UNKNOWN_TOKEN);
            mp_cond_format = m_sheet.start_cond_format();
            if (mp_cond_format)
                std::for_each(attrs.begin(), attrs.end(), conditional_formatting_attr_parser(*mp_cond_format, m_resolver));
            break;
        }
        case XML_cfRule:
        {
            xml_element_expected(parent, NS_ooxml_xlsx, XML_conditionalFormatting);
            if (mp_cond_format)
            {
                cfRule_attr_parser parser = std::for_each(attrs.begin(), attrs.end(), cfRule_attr_parser());
                start_rule(parser);
            }
            break;
        }
        case XML_cfvo:
        {
            cfvo_attr_parser parser = std::for_each(attrs.begin(), attrs.end(), cfvo_attr_parser(m_pool));
            m_cfvo_values.push_back(parser.get_values());
            break;
        }
        case XML_dataBar:
        {
            xml_element_expected(parent, NS_ooxml_xlsx, XML_cfRule);
            data_bar_attr_parser parser = std::for_each(attrs.begin(), attrs.end(), data_bar_attr_parser());
            if (mp_databar)
                parser.import_data(*mp_databar);
            break;
        }
        case XML_iconSet:
        {
            xml_element_expected(parent, NS_ooxml_xlsx, XML_cfRule);
            icon_set_attr_parser parser = std::for_each(attrs.begin(), attrs.end(), icon_set_attr_parser());
            if (mp_iconset)
                parser.import_data(*mp_iconset);
            break;
        }
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
                mp_databar->set_color_negative(color.alpha, color.red, color.green, color.blue);

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
                std::vector<argb_color>::const_iterator itrColor = m_colors.begin();
                for (std::vector<cfvo_values>::const_iterator itr = m_cfvo_values.begin(); itr != m_cfvo_values.end(); ++itr, ++itrColor)
                {
                    auto* value = mp_colorscale->start_value();
                    ENSURE_INTERFACE(value, import_cond_format_value);

                    import_cfvo(*itr, *value);
                    value->set_color(itrColor->alpha, itrColor->red, itrColor->green, itrColor->blue);
                    value->commit();
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

void xlsx_conditional_format_context::start_rule(const cfRule_attr_parser& attrs)
{
    switch (attrs.get_type())
    {
        case timePeriod:
        {
            mp_date = mp_cond_format->start_date();
            ENSURE_INTERFACE(mp_date, import_cond_format_date);

            mp_date->set_period(attrs.get_period());

            if (attrs.get_dxf_id())
                mp_date->set_dxf_id(*attrs.get_dxf_id());
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

            mp_condition->set_operator(attrs.get_operator());

            if (attrs.get_dxf_id())
                mp_condition->set_dxf_id(*attrs.get_dxf_id());

            if (!attrs.get_rank().empty())
                mp_condition->set_rank(attrs.get_rank());

            if (!attrs.get_text().empty())
                mp_condition->set_text(attrs.get_text());

            if (!attrs.get_std_dev().empty())
                mp_condition->set_std_dev(attrs.get_std_dev());
            break;
        }
    }
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
