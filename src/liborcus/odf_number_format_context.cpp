/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "odf_number_format_context.hpp"
#include "odf_namespace_types.hpp"
#include "odf_token_constants.hpp"
#include "odf_helper.hpp"
#include "ods_session_data.hpp"
#include "impl_utils.hpp"

#include <orcus/measurement.hpp>
#include <orcus/spreadsheet/import_interface.hpp>
#include <orcus/spreadsheet/import_interface_styles.hpp>
#include <orcus/spreadsheet/styles.hpp>

#include <iostream>
#include <algorithm>
#include <string>

namespace ss = orcus::spreadsheet;

namespace orcus {

namespace {

enum class date_style_type
{
    unknown = 0,
    short_symbol,
    long_symbol
};

date_style_type to_date_style(std::string_view s)
{
    constexpr std::pair<std::string_view, date_style_type> entries[] = {
        { "short", date_style_type::short_symbol },
        { "long", date_style_type::long_symbol },
    };

    for (const auto& entry : entries)
    {
        if (s == entry.first)
            return entry.second;
    }

    return date_style_type::unknown;
}

struct parse_result
{
    bool success = true;
    std::string error_message;
};

date_style_type parse_attrs_for_date_style(const std::vector<xml_token_attr_t>& attrs)
{
    for (const auto& attr : attrs)
    {
        if (attr.ns == NS_odf_number)
        {
            switch (attr.name)
            {
                case XML_style:
                    return to_date_style(attr.value);
            }
        }
    }

    return date_style_type::unknown;
}

void parse_element_time_short_long(const std::vector<xml_token_attr_t>& attrs, char c, odf_number_format& style)
{
    style.code += c;

    if (parse_attrs_for_date_style(attrs) == date_style_type::long_symbol)
        style.code += c;
}

void parse_element_number(const std::vector<xml_token_attr_t>& attrs, odf_number_format& style)
{
    bool grouping = false;
    long decimal_places = 0;
    long min_integer_digits = 0;

    for (const auto& attr : attrs)
    {
        if (attr.ns == NS_odf_number)
        {
            switch (attr.name)
            {
                case XML_decimal_places:
                {
                    decimal_places = to_long(attr.value);
                    break;
                }
                case XML_grouping:
                    grouping = to_bool(attr.value);
                    break;
                case XML_min_integer_digits:
                    min_integer_digits = to_long(attr.value);
                    break;
                default:;
            }
        }
    }

    if (grouping)
    {
        if (min_integer_digits < 4)
        {
            style.code += "#,";

            for (long i = 0; i < 3 - min_integer_digits; ++i)
                style.code += "#";

            for (long i = 0; i < min_integer_digits; ++i)
                style.code += "0";
        }
        else
        {
            std::string temporary_code;

            for (long i = 0; i < min_integer_digits; ++i)
            {
                if (i % 3 == 0 && i != 0)
                    temporary_code += ",";

                temporary_code += "0";
            }

            std::reverse(temporary_code.begin(), temporary_code.end());
            style.code += temporary_code;
        }
    }
    else
    {
        if (min_integer_digits == 0)
            style.code += "#";

        for (long i = 0; i < min_integer_digits; ++i)
            style.code += "0";
    }

    if (decimal_places > 0)
    {
        style.code += ".";
        for (long i = 0; i < decimal_places; ++i)
            style.code += "0";
    }
}

void parse_element_text_properties(const std::vector<xml_token_attr_t>& attrs, odf_number_format& style)
{
    std::string_view color;

    for (const auto& attr : attrs)
    {
        if (attr.ns == NS_odf_fo)
        {
            switch (attr.name)
            {
                case XML_color:
                {
                    if (attr.value == "#000000")
                        color = "BLACK";
                    if (attr.value == "#ff0000")
                        color = "RED";
                    if (attr.value == "#00ff00")
                        color = "GREEN";
                    if (attr.value == "#0000ff")
                        color = "BLUE";
                    if (attr.value == "#ffff00")
                        color = "YELLOW";
                    if (attr.value == "#00ffff")
                        color = "CYAN";
                    if (attr.value == "#ff00ff")
                        color = "MAGENTA";
                    if (attr.value == "#ffffff")
                        color = "WHITE";
                }
            }
        }
    }

    if (!color.empty())
    {
        std::ostringstream os;
        os << '[' << color << ']';
        style.code += os.str();
    }
}

parse_result parse_element_map(session_context& cxt, const std::vector<xml_token_attr_t>& attrs, odf_number_format& style)
{
    parse_result res;

    std::string_view comp; // comparison operator e.g. <, >, >=, ...
    std::string_view value; // right-hand value
    std::string_view style_name; // style name associated with the mapped rule

    for (const auto& attr : attrs)
    {
        if (attr.ns == NS_odf_style)
        {
            switch (attr.name)
            {
                case XML_apply_style_name:
                {
                    style_name = attr.value;
                    break;
                }
                case XML_condition:
                {
                    // value()[comp][rvalue] e.g. 'value()>=0'
                    constexpr std::string_view prefix = "value()";

                    // check if the attribute value starts with 'value()'
                    if (attr.value.compare(0, prefix.size(), prefix) == 0)
                    {
                        auto pos_value = attr.value.find_first_not_of("<>=", prefix.size());

                        comp = attr.value.substr(prefix.size(), pos_value - prefix.size());
                        value = attr.value.substr(pos_value);
                    }
                    break;
                }
            }
        }
    }

    if (comp.empty() || value.empty() || style_name.empty())
    {
        res.success = false;
        return res;
    }

    // fetch the code associated with the mapped rule
    auto& numfmts = cxt.get_data<ods_session_data>().number_formats;
    std::string_view code = numfmts.get_code(style_name);

    if (code.empty())
    {
        res.success = false;
        std::ostringstream os;
        os << "code stored for the number format style named '" << style_name << "' exists, but is empty.";
        res.error_message = os.str();
        return res;
    }

    // prepend the mapped rule to the current code
    std::ostringstream os;
    os << '[' << comp << value << ']' << code << ';' << style.code;
    style.code = os.str();

    return res;
}

} // anonymous namespace

date_style_context::date_style_context(session_context& session_cxt, const tokens& tk) :
    xml_context_base(session_cxt, tk)
{
    static const xml_element_validator::rule rules[] = {
        // parent element -> child element
        { XMLNS_UNKNOWN_ID, XML_UNKNOWN_TOKEN, NS_odf_number, XML_date_style }, // root element
        { NS_odf_number, XML_date_style, NS_odf_number, XML_day },
        { NS_odf_number, XML_date_style, NS_odf_number, XML_month },
        { NS_odf_number, XML_date_style, NS_odf_number, XML_text },
        { NS_odf_number, XML_date_style, NS_odf_number, XML_year },
    };

    init_element_validator(rules, std::size(rules));
}

void date_style_context::start_element(xmlns_id_t ns, xml_token_t name, const std::vector<xml_token_attr_t>& attrs)
{
    push_stack(ns, name);

    if (ns == NS_odf_number)
    {
        switch (name)
        {
            case XML_date_style:
                start_element_date_style(attrs);
                break;
            case XML_month:
                start_element_month(attrs);
                break;
            case XML_day:
                start_element_day(attrs);
                break;
            case XML_year:
                start_element_year(attrs);
                break;
            case XML_text:
                m_text_stream = std::ostringstream{};
                break;
            default:
                warn_unhandled();
        }
    }
    else
        warn_unhandled();
}

bool date_style_context::end_element(xmlns_id_t ns, xml_token_t name)
{
    if (ns == NS_odf_number)
    {
        switch (name)
        {
            case XML_text:
                m_current_style->code += m_text_stream.str();
                break;
        }
    }
    return pop_stack(ns, name);
}

void date_style_context::characters(std::string_view str, bool /*transient*/)
{
    m_text_stream << str;
}

void date_style_context::reset()
{
    m_current_style = std::make_unique<odf_number_format>();
}

std::unique_ptr<odf_number_format> date_style_context::pop_style()
{
    return std::move(m_current_style);
}

void date_style_context::start_element_date_style(const std::vector<xml_token_attr_t>& attrs)
{
    for (const auto& attr : attrs)
    {
        if (attr.ns == NS_odf_style)
        {
            switch (attr.name)
            {
                case XML_name:
                    m_current_style->name = intern(attr);
                    break;
            }
        }
    }
}

void date_style_context::start_element_month(const std::vector<xml_token_attr_t>& attrs)
{
    auto style = date_style_type::unknown;
    bool textual = false;

    for (const auto& attr : attrs)
    {
        if (attr.ns == NS_odf_number)
        {
            switch (attr.name)
            {
                case XML_style:
                    style = to_date_style(attr.value);
                    break;
                case XML_textual:
                    textual = to_bool(attr.value);
                    break;
            }
        }
    }

    m_current_style->code += 'M';

    if (style == date_style_type::long_symbol)
        m_current_style->code += 'M';

    if (textual)
        m_current_style->code += 'M';

    if (style == date_style_type::long_symbol && textual)
        m_current_style->code += 'M';
}

void date_style_context::start_element_day(const std::vector<xml_token_attr_t>& attrs)
{
    m_current_style->code += 'D';

    if (parse_attrs_for_date_style(attrs) == date_style_type::long_symbol)
        m_current_style->code += 'D';
}

void date_style_context::start_element_year(const std::vector<xml_token_attr_t>& attrs)
{
    m_current_style->code += "YY";

    if (parse_attrs_for_date_style(attrs) == date_style_type::long_symbol)
        m_current_style->code += "YY";
}

time_style_context::time_style_context(session_context& session_cxt, const tokens& tk) :
    xml_context_base(session_cxt, tk)
{
    static const xml_element_validator::rule rules[] = {
        // parent element -> child element
        { XMLNS_UNKNOWN_ID, XML_UNKNOWN_TOKEN, NS_odf_number, XML_time_style }, // root element
        { NS_odf_number, XML_time_style, NS_odf_number, XML_hours },
        { NS_odf_number, XML_time_style, NS_odf_number, XML_minutes },
        { NS_odf_number, XML_time_style, NS_odf_number, XML_seconds },
        { NS_odf_number, XML_time_style, NS_odf_number, XML_text },
        { NS_odf_number, XML_time_style, NS_odf_number, XML_am_pm },
    };

    init_element_validator(rules, std::size(rules));
}

void time_style_context::start_element(xmlns_id_t ns, xml_token_t name, const std::vector<xml_token_attr_t>& attrs)
{
    push_stack(ns, name);

    if (ns == NS_odf_number)
    {
        switch (name)
        {
            case XML_time_style:
                start_element_time_style(attrs);
                break;
            case XML_hours:
                parse_element_time_short_long(attrs, 'H', *m_current_style);
                break;
            case XML_minutes:
                parse_element_time_short_long(attrs, 'M', *m_current_style);
                break;
            case XML_seconds:
                start_element_seconds(attrs);
                break;
            case XML_text:
                m_text_stream = std::ostringstream{};
                break;
            case XML_am_pm:
                m_current_style->code += "AM/PM";
                break;
            default:
                warn_unhandled();
        }
    }
    else
        warn_unhandled();
}

bool time_style_context::end_element(xmlns_id_t ns, xml_token_t name)
{
    if (ns == NS_odf_number)
    {
        switch (name)
        {
            case XML_text:
                m_current_style->code += m_text_stream.str();
                break;
        }
    }

    return pop_stack(ns, name);
}

void time_style_context::characters(std::string_view str, bool /*transient*/)
{
    m_text_stream << str;
}

void time_style_context::reset()
{
    m_current_style = std::make_unique<odf_number_format>();
}

std::unique_ptr<odf_number_format> time_style_context::pop_style()
{
    return std::move(m_current_style);
}

void time_style_context::start_element_time_style(const std::vector<xml_token_attr_t>& attrs)
{
    for (const auto& attr : attrs)
    {
        if (attr.ns == NS_odf_style && attr.name == XML_name)
            m_current_style->name = intern(attr);
    }
}

void time_style_context::start_element_seconds(const std::vector<xml_token_attr_t>& attrs)
{
    auto style = date_style_type::unknown;
    std::optional<long> decimal_places;

    for (const auto& attr : attrs)
    {
        if (attr.ns == NS_odf_number)
        {
            switch (attr.name)
            {
                case XML_style:
                    style = to_date_style(attr.value);
                    break;
                case XML_decimal_places:
                    decimal_places = to_long(attr.value);
                    break;
            }
        }
    }

    m_current_style->code += 'S';

    if (style == date_style_type::long_symbol)
        m_current_style->code += 'S';

    if (decimal_places && *decimal_places > 0)
        m_current_style->code += std::string{'S', *decimal_places};
}

percentage_style_context::percentage_style_context(session_context& session_cxt, const tokens& tk) :
    xml_context_base(session_cxt, tk)
{
    static const xml_element_validator::rule rules[] = {
        // parent element -> child element
        { XMLNS_UNKNOWN_ID, XML_UNKNOWN_TOKEN, NS_odf_number, XML_percentage_style }, // root element
        { NS_odf_number, XML_percentage_style, NS_odf_number, XML_number },
        { NS_odf_number, XML_percentage_style, NS_odf_number, XML_text },
    };

    init_element_validator(rules, std::size(rules));
}

void percentage_style_context::start_element(xmlns_id_t ns, xml_token_t name, const std::vector<xml_token_attr_t>& attrs)
{
    push_stack(ns, name);

    if (ns == NS_odf_number)
    {
        switch (name)
        {
            case XML_percentage_style:
            {
                for (const auto& attr : attrs)
                {
                    if (attr.ns == NS_odf_style && attr.name == XML_name)
                        m_current_style->name = intern(attr);
                }
                break;
            }
            case XML_number:
            {
                parse_element_number(attrs, *m_current_style);
                break;
            }
            case XML_text:
                m_text_stream = std::ostringstream{};
                break;
            default:
                warn_unhandled();
        }
    }
    else
        warn_unhandled();
}

bool percentage_style_context::end_element(xmlns_id_t ns, xml_token_t name)
{
    if (ns == NS_odf_number)
    {
        switch (name)
        {
            case XML_text:
                m_current_style->code += m_text_stream.str();
                break;
        }
    }
    return pop_stack(ns, name);
}

void percentage_style_context::characters(std::string_view str, bool /*transient*/)
{
    m_text_stream << str;
}

void percentage_style_context::reset()
{
    m_current_style = std::make_unique<odf_number_format>();
}

std::unique_ptr<odf_number_format> percentage_style_context::pop_style()
{
    return std::move(m_current_style);
}

boolean_style_context::boolean_style_context(session_context& session_cxt, const tokens& tk) :
    xml_context_base(session_cxt, tk)
{
    static const xml_element_validator::rule rules[] = {
        // parent element -> child element
        { XMLNS_UNKNOWN_ID, XML_UNKNOWN_TOKEN, NS_odf_number, XML_boolean_style }, // root element
        { NS_odf_number, XML_boolean_style, NS_odf_number, XML_boolean },
    };

    init_element_validator(rules, std::size(rules));
}

void boolean_style_context::start_element(xmlns_id_t ns, xml_token_t name, const std::vector<xml_token_attr_t>& attrs)
{
    push_stack(ns, name);

    if (ns == NS_odf_number)
    {
        switch (name)
        {
            case XML_boolean_style:
            {
                for (const auto& attr : attrs)
                {
                    if (attr.ns == NS_odf_style && attr.name == XML_name)
                        m_current_style->name = intern(attr);
                }
                break;
            }
            case XML_boolean:
            {
                m_current_style->code += "BOOLEAN";
                break;
            }
            default:
                warn_unhandled();
        }
    }
    else
        warn_unhandled();
}

bool boolean_style_context::end_element(xmlns_id_t ns, xml_token_t name)
{
    return pop_stack(ns, name);
}

void boolean_style_context::reset()
{
    m_current_style = std::make_unique<odf_number_format>();
}

std::unique_ptr<odf_number_format> boolean_style_context::pop_style()
{
    return std::move(m_current_style);
}

text_style_context::text_style_context(session_context& session_cxt, const tokens& tk) :
    xml_context_base(session_cxt, tk)
{
    static const xml_element_validator::rule rules[] = {
        // parent element -> child element
        { XMLNS_UNKNOWN_ID, XML_UNKNOWN_TOKEN, NS_odf_number, XML_text_style }, // root element
        { NS_odf_number, XML_text_style, NS_odf_number, XML_text_content },
    };

    init_element_validator(rules, std::size(rules));
}

void text_style_context::start_element(xmlns_id_t ns, xml_token_t name, const std::vector<xml_token_attr_t>& attrs)
{
    push_stack(ns, name);

    if (ns == NS_odf_number)
    {
        switch (name)
        {
            case XML_text_style:
            {
                for (const auto& attr : attrs)
                {
                    if (attr.ns == NS_odf_style && attr.name == XML_name)
                        m_current_style->name = intern(attr);
                }
                break;
            }
            case XML_text_content:
            {
                m_current_style->code += '@';
                break;
            }
            default:
                warn_unhandled();
        }
    }
    else
        warn_unhandled();
}

bool text_style_context::end_element(xmlns_id_t ns, xml_token_t name)
{
    return pop_stack(ns, name);
}

void text_style_context::reset()
{
    m_current_style = std::make_unique<odf_number_format>();
}

std::unique_ptr<odf_number_format> text_style_context::pop_style()
{
    return std::move(m_current_style);
}

number_style_context::number_style_context(session_context& session_cxt, const tokens& tk) :
    xml_context_base(session_cxt, tk)
{
    static const xml_element_validator::rule rules[] = {
        // parent element -> child element
        { XMLNS_UNKNOWN_ID, XML_UNKNOWN_TOKEN, NS_odf_number, XML_number_style }, // root element
        { NS_odf_number, XML_number_style, NS_odf_number, XML_fraction },
        { NS_odf_number, XML_number_style, NS_odf_number, XML_number },
        { NS_odf_number, XML_number_style, NS_odf_number, XML_scientific_number },
        { NS_odf_number, XML_number_style, NS_odf_number, XML_text },
        { NS_odf_number, XML_number_style, NS_odf_style, XML_map },
        { NS_odf_number, XML_number_style, NS_odf_style, XML_text_properties },
    };

    init_element_validator(rules, std::size(rules));
}

void number_style_context::start_element(xmlns_id_t ns, xml_token_t name, const std::vector<xml_token_attr_t>& attrs)
{
    push_stack(ns, name);

    if (ns == NS_odf_number)
    {
        switch (name)
        {
            case XML_fraction:
                start_element_fraction(attrs);
                break;
            case XML_number_style:
                start_element_number_style(attrs);
                break;
            case XML_number:
                parse_element_number(attrs, *m_current_style);
                break;
            case XML_scientific_number:
                start_element_scientific_number(attrs);
                break;
            case XML_text:
                m_text_stream = std::ostringstream{};
                break;
            default:
                warn_unhandled();
        }
    }
    else if (ns == NS_odf_style)
    {
        switch (name)
        {
            case XML_text_properties:
                parse_element_text_properties(attrs, *m_current_style);
                break;
            case XML_map:
            {
                auto res = parse_element_map(get_session_context(), attrs, *m_current_style);
                if (!res.success && get_config().debug)
                    warn(res.error_message);

                break;
            }
            default:
                warn_unhandled();
        }
    }
    else
        warn_unhandled();
}

bool number_style_context::end_element(xmlns_id_t ns, xml_token_t name)
{
    if (ns == NS_odf_number)
    {
        switch (name)
        {
            case XML_text:
                m_current_style->code += m_text_stream.str();
                break;
            default:;
        }
    }

    return pop_stack(ns, name);
}

void number_style_context::characters(std::string_view str, bool /*transient*/)
{
    m_text_stream << str;
}

void number_style_context::reset()
{
    m_current_style = std::make_unique<odf_number_format>();
    m_text_stream = std::ostringstream{};
    m_country_code = std::string_view{};
    m_language = std::string_view{};
}

std::unique_ptr<odf_number_format> number_style_context::pop_style()
{
    return std::move(m_current_style);
}

void number_style_context::start_element_fraction(const std::vector<xml_token_attr_t>& attrs)
{
    long min_integer_digits = 0;
    long min_numerator_digits = 0;
    long min_denominator_digits = 0;

    std::optional<std::string_view> denominator_value;

    for (const auto& attr : attrs)
    {
        if (attr.ns == NS_odf_number)
        {
            switch (attr.name)
            {
                case XML_min_integer_digits:
                    min_integer_digits = to_long(attr.value);
                    break;
                case XML_min_numerator_digits:
                    min_numerator_digits = to_long(attr.value);
                    break;
                case XML_min_denominator_digits:
                    min_denominator_digits = to_long(attr.value);
                    break;
                case XML_denominator_value:
                {
                    denominator_value = attr.value;
                    break;
                }
            }
        }
    }

    if (min_integer_digits > 0)
    {
        m_current_style->code += std::string{'#', min_integer_digits};
        m_current_style->code += ' ';
    }

    if (min_numerator_digits > 0)
        m_current_style->code += std::string{'?', min_numerator_digits};

    m_current_style->code += '/';

    if (denominator_value)
        m_current_style->code += *denominator_value;
    else if (min_denominator_digits > 0)
        m_current_style->code += std::string{'?', min_denominator_digits};
}

void number_style_context::start_element_number_style(const std::vector<xml_token_attr_t>& attrs)
{
    for (const xml_token_attr_t& attr: attrs)
    {
        if (attr.ns == NS_odf_number)
        {
            switch (attr.name)
            {
                case XML_country:
                    m_country_code = attr.value;
                    break;
                case XML_language:
                    m_language = attr.value;
                    break;
                default:
                    ;
            }
        }
        else if (attr.ns == NS_odf_style)
        {
            switch (attr.name)
            {
                case XML_name:
                    m_current_style->name = attr.value;
                    break;
                default:
                    ;
            }
        }
    }
}

void number_style_context::start_element_scientific_number(const std::vector<xml_token_attr_t>& attrs)
{
    long decimal_places = 0;
    long min_exponent_digits = 0;
    long min_integer_digits = 0;
    bool grouping = false;

    for (const xml_token_attr_t& attr : attrs)
    {
        if (attr.ns == NS_odf_number)
        {
            switch (attr.name)
            {
                case XML_decimal_places:
                    decimal_places = to_long(attr.value);
                    break;
                case XML_grouping:
                    grouping = to_bool(attr.value);
                    break;
                case XML_min_exponent_digits:
                    min_exponent_digits = to_long(attr.value);
                    break;
                case XML_min_integer_digits:
                    min_integer_digits = to_long(attr.value);
                    break;
            }
        }
    }

    if (grouping)
    {
        if (min_integer_digits < 4)
        {
            m_current_style->code += "#,";

            for (long i = 0; i < 3 - min_integer_digits; ++i)
            {
                m_current_style->code += "#";
            }

            for (long i = 0; i < min_integer_digits; ++i)
            {
                m_current_style->code += "0";
            }
        }
        else
        {
            std::string temporary_code;
            for (long i = 0; i < min_integer_digits; ++i)
            {
                if (i % 3 == 0 && i != 0)
                    temporary_code += ",";

                temporary_code += "0";
            }

            std::reverse(temporary_code.begin(), temporary_code.end());
            m_current_style->code += temporary_code;
        }
    }
    else
    {
        if (min_integer_digits == 0)
            m_current_style->code += "#";

        for (long i = 0; i < min_integer_digits; ++i)
            m_current_style->code += "0";
    }

    m_current_style->code += ".";

    for (long i = 0; i < decimal_places; ++i)
        m_current_style->code += "0";

    m_current_style->code += "E+";

    for (long i = 0; i < min_exponent_digits; ++i)
        m_current_style->code += "0";
}

currency_style_context::currency_style_context(session_context& session_cxt, const tokens& tk) :
    xml_context_base(session_cxt, tk)
{
    static const xml_element_validator::rule rules[] = {
        // parent element -> child element
        { XMLNS_UNKNOWN_ID, XML_UNKNOWN_TOKEN, NS_odf_number, XML_currency_style }, // root element
        { NS_odf_number, XML_currency_style, NS_odf_number, XML_currency_symbol },
        { NS_odf_number, XML_currency_style, NS_odf_number, XML_number },
        { NS_odf_number, XML_currency_style, NS_odf_number, XML_text },
        { NS_odf_number, XML_currency_style, NS_odf_style, XML_map },
        { NS_odf_number, XML_currency_style, NS_odf_style, XML_text_properties },
    };

    init_element_validator(rules, std::size(rules));
}

void currency_style_context::start_element(xmlns_id_t ns, xml_token_t name, const std::vector<xml_token_attr_t>& attrs)
{
    push_stack(ns, name);

    if (ns == NS_odf_number)
    {
        switch (name)
        {
            case XML_currency_style:
                start_element_currency_style(attrs);
                break;
            case XML_currency_symbol:
                m_text_stream = std::ostringstream{};
                break;
            case XML_number:
                parse_element_number(attrs, *m_current_style);
                break;
            case XML_text:
                m_text_stream = std::ostringstream{};
                break;
            default:
                warn_unhandled();
        }
    }
    else if (ns == NS_odf_style)
    {
        switch (name)
        {
            case XML_map:
            {
                auto res = parse_element_map(get_session_context(), attrs, *m_current_style);
                if (!res.success && get_config().debug)
                    warn(res.error_message);

                break;
            }
            case XML_text_properties:
                parse_element_text_properties(attrs, *m_current_style);
                break;
            default:
                warn_unhandled();
        }
    }
    else
        warn_unhandled();
}

bool currency_style_context::end_element(xmlns_id_t ns, xml_token_t name)
{
    if (ns == NS_odf_number)
    {
        switch (name)
        {
            case XML_currency_symbol:
            {
                std::ostringstream os;
                os << m_current_style->code << "[$" << m_text_stream.str() << ']';
                m_current_style->code = os.str();
                break;
            }
            case XML_text:
                m_current_style->code += m_text_stream.str();
                break;
        }
    }

    return pop_stack(ns, name);
}

void currency_style_context::characters(std::string_view str, bool /*transient*/)
{
    m_text_stream << str;
}

void currency_style_context::reset()
{
    m_current_style = std::make_unique<odf_number_format>();
}

std::unique_ptr<odf_number_format> currency_style_context::pop_style()
{
    return std::move(m_current_style);
}

void currency_style_context::start_element_currency_style(const std::vector<xml_token_attr_t>& attrs)
{
    for (const auto& attr : attrs)
    {
        if (attr.ns == NS_odf_style)
        {
            switch (attr.name)
            {
                case XML_name:
                    m_current_style->name = intern(attr);
                    break;
                case XML_volatile:
                    m_current_style->is_volatile = to_bool(attr.value);
                    break;
            }
        }
        else if (attr.ns == NS_odf_number)
        {
            switch (attr.name)
            {
                case XML_language:
                    m_language = intern(attr);
                    break;
                case XML_country:
                    m_country_code = intern(attr);
                    break;
            }
        }
    }
}

} // namespace orcus

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
