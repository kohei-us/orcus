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

struct parse_result
{
    bool success = true;
    std::string error_message;
};

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

namespace {

class number_style_attr_parser
{
    std::string_view m_country_code;
    std::string_view m_style_name;
    std::string_view m_language;
    bool m_volatile;

public:

    number_style_attr_parser():
        m_volatile(false)
    {}

    void operator() (const xml_token_attr_t& attr)
    {
        if (attr.ns == NS_odf_number)
        {
            switch(attr.name)
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
                    m_style_name = attr.value;
                break;
                case XML_volatile:
                    m_volatile = attr.value == "true";
                break;
                default:
                    ;
            }
        }
    }

    std::string_view get_style_name() const { return m_style_name;}
    std::string_view get_country_code() const { return m_country_code;}
    bool is_volatile() const { return m_volatile;}
    std::string_view get_language() const { return m_language;}
};

class number_attr_parser
{
    size_t m_decimal_places;
    size_t m_min_int_digits;
    bool m_grouping;

public:

    number_attr_parser() :
        m_decimal_places(0),
        m_min_int_digits(0),
        m_grouping(false)
    {}

    void operator() (const xml_token_attr_t& attr)
    {
        if (attr.ns == NS_odf_number)
        {
            switch (attr.name)
            {
                case XML_decimal_places:
                {
                    m_decimal_places = to_long(attr.value);
                }
                break;
                case XML_grouping:
                    m_grouping = attr.value == "true";
                break;
                case XML_min_integer_digits:
                    m_min_int_digits = to_long(attr.value);
                break;
                default:
                    ;
            }
        }
    }

    size_t get_decimal_places() const { return m_decimal_places;}
    bool is_grouped() const { return m_grouping;}
    size_t get_min_int_digits() const { return m_min_int_digits;}
    bool has_decimal_places() const { return m_decimal_places > 0;}
};

class scientific_number_attr_parser
{
    size_t m_decimal_places;
    bool m_grouping;
    size_t m_min_exp_digits;
    size_t m_min_int_digits;

public:

    scientific_number_attr_parser() :
        m_decimal_places(0),
        m_grouping(false),
        m_min_exp_digits(0),
        m_min_int_digits(0)
    {}

    void operator() (const xml_token_attr_t& attr)
    {
        if (attr.ns == NS_odf_number)
        {
            switch(attr.name)
            {
                case XML_decimal_places:
                    m_decimal_places = to_long(attr.value);
                break;
                case XML_grouping:
                    m_grouping = attr.value == "true";
                break;
                case XML_min_exponent_digits:
                    m_min_exp_digits = to_long(attr.value);
                break;
                case XML_min_integer_digits:
                    m_min_int_digits = to_long(attr.value);
                break;
                default:
                    ;
            }
        }
    }

    size_t get_decimal_places() const { return m_decimal_places;}
    bool is_grouped() const { return m_grouping;}
    size_t get_min_exp_digits() const { return m_min_exp_digits;}
    size_t get_min_int_digits() const { return m_min_int_digits;}
};

class generic_style_attr_parser
{
    std::string_view m_style_name;
    bool m_volatile;
    bool m_long;

public:
    generic_style_attr_parser() :
        m_volatile(false),
        m_long(false)
    {}

    void operator() (const xml_token_attr_t& attr)
    {
        if (attr.ns == NS_odf_style)
        {
            switch (attr.name)
            {
                case XML_name:
                    m_style_name = attr.value;
                break;
                case XML_volatile:
                    m_volatile = attr.value == "true";
                break;
                default:
                    ;
            }
        }
        else if (attr.ns == NS_odf_number)
            if (attr.name == XML_style)
                m_long = attr.value == "long";
    }

    std::string_view get_style_name() const { return m_style_name;}
    bool is_volatile() const { return m_volatile;}
    bool has_long() const { return m_long;}
};

class month_attr_parser
{
    bool m_style_name;
    bool m_textual;

public:
    month_attr_parser():
        m_style_name(false),
        m_textual(false)
    {}

    void operator() (const xml_token_attr_t& attr)
    {
        if (attr.ns == NS_odf_number)
        {
            if (attr.name == XML_style)
                m_style_name = attr.value == "long";
            if (attr.name == XML_textual)
                m_textual = attr.value == "true";
        }
    }

    bool has_long() const { return m_style_name;}
    bool is_textual() const { return m_textual;}
};

class seconds_attr_parser
{
    size_t m_decimal_places;
    bool m_style_name;

public:
    seconds_attr_parser():
        m_decimal_places(0),
        m_style_name(false)
    {}

    void operator() (const xml_token_attr_t& attr)
    {
        if (attr.ns == NS_odf_number)
        {
            if (attr.name == XML_style)
                m_style_name = attr.value == "long";
            if (attr.name == XML_decimal_places)
            {
                m_decimal_places = to_long(attr.value);
            }
        }
    }

    bool has_long() const { return m_style_name;}
    size_t get_decimal_places() const { return m_decimal_places;}
    bool has_decimal_places() const { return m_decimal_places > 0;}
};

class fraction_attr_parser
{
    size_t m_min_int_digits;
    size_t m_min_deno_digits;
    size_t m_min_num_digits;
    std::string_view m_deno_value;

    bool m_predefined_deno;

public:
    fraction_attr_parser():
        m_min_int_digits(0),
        m_min_deno_digits(0),
        m_min_num_digits(0),
        m_predefined_deno(false)
    {}

    void operator() (const xml_token_attr_t& attr)
    {
        if (attr.ns == NS_odf_number)
        {
            switch(attr.name)
            {
                case XML_min_integer_digits:
                    m_min_int_digits = to_long(attr.value);
                break;
                case XML_min_numerator_digits:
                    m_min_num_digits = to_long(attr.value);
                break;
                case XML_min_denominator_digits:
                    m_min_deno_digits = to_long(attr.value);
                break;
                case XML_denominator_value:
                {
                    m_deno_value = attr.value;
                    m_predefined_deno = true;
                }
                break;
                default:
                    ;
            }
        }
    }

    size_t get_min_int_digits() const { return m_min_int_digits;}
    size_t get_min_num_digits() const { return m_min_num_digits;}
    size_t get_min_deno_digits() const { return m_min_deno_digits;}
    std::string_view get_deno_value() const { return m_deno_value;}
    bool has_predefined_deno() const { return m_predefined_deno;}
};

class text_properties_attr_parser
{
    std::string_view m_color;
    bool color_absent;

public:
    text_properties_attr_parser():
        color_absent(true)
    {}

    void operator() (const xml_token_attr_t& attr)
    {
        if (attr.ns == NS_odf_fo)
        {
            switch (attr.name)
            {
                case XML_color:
                {
                    if (attr.value == "#000000")
                        m_color = "BLACK";
                    if (attr.value == "#ff0000")
                        m_color = "RED";
                    if (attr.value == "#00ff00")
                        m_color = "GREEN";
                    if (attr.value == "#0000ff")
                        m_color = "BLUE";
                    if (attr.value == "#ffff00")
                        m_color = "YELLOW";
                    if (attr.value == "#00ffff")
                        m_color = "CYAN";
                    if (attr.value == "#ff00ff")
                        m_color = "MAGENTA";
                    if (attr.value == "#ffffff")
                        m_color = "WHITE";
                    else
                        color_absent = false;
                }
            }
        }
    }

    std::string_view get_color() const { return m_color;}
    bool has_color() const { return !color_absent;}
};

class map_attr_parser
{
    std::string m_value;
    std::string m_sign;
    bool m_has_map;

public:
    map_attr_parser():
        m_has_map(false)
    {}

    void operator() (const xml_token_attr_t& attr)
    {
        if (attr.ns == NS_odf_style)
        {
            if (attr.name == XML_condition)
            {
                for (size_t i = 0; i < attr.value.size(); i++)
                {
                    if (attr.value[i] == '<' || attr.value[i] == '>' || attr.value[i] == '=')
                        m_sign = m_sign + attr.value[i];
                    if (isdigit(attr.value[i]))
                        m_value = m_value + attr.value[i];
                }
                m_has_map = true;
            }
        }
    }
    std::string get_value() const { return m_value;}
    std::string get_sign() const { return m_sign;}
    bool has_map() const { return m_has_map;}
};

}

number_format_context::number_format_context(
    session_context& session_cxt, const tokens& tk, ss::iface::import_styles* iface_styles) :
    xml_context_base(session_cxt, tk),
    mp_styles(iface_styles)
{}

xml_context_base* number_format_context::create_child_context(xmlns_id_t /*ns*/, xml_token_t /*name*/)
{
    return nullptr;
}

void number_format_context::end_child_context(xmlns_id_t /*ns*/, xml_token_t /*name*/, xml_context_base* /*child*/)
{
}

void number_format_context::start_element(xmlns_id_t ns, xml_token_t name, const std::vector<xml_token_attr_t>& attrs)
{
    xml_token_pair_t parent = push_stack(ns, name);
    (void)parent;

    m_character_stream = std::string_view{};

    if (ns == NS_odf_number)
    {
        switch(name)
        {
            case XML_number_style:
            {
                number_style_attr_parser func;
                func = std::for_each(attrs.begin(), attrs.end(), func);
                m_current_style.name = func.get_style_name();
                m_current_style.is_volatile = func.is_volatile();
                break;
            }
            case XML_number:
            {
                number_attr_parser func;
                func = std::for_each(attrs.begin(), attrs.end(), func);
                if (func.is_grouped())
                {
                    if (func.get_min_int_digits() < 4)
                    {
                        m_current_style.code += "#,";
                        for (size_t i = 0; i < 3 - func.get_min_int_digits(); i++)
                        {
                            m_current_style.code += "#";
                        }
                        for (size_t i = 0; i < func.get_min_int_digits(); i++)
                        {
                            m_current_style.code += "0";
                        }
                    }
                    else
                    {
                        std::string temporary_code;
                        for(size_t i = 0; i < func.get_min_int_digits(); i++)
                        {
                            if (i % 3 == 0 && i != 0)
                                temporary_code += ",";
                            temporary_code += "0";
                        }
                        std::reverse(temporary_code.begin(), temporary_code.end());
                        m_current_style.code += temporary_code;
                    }
                }
                else
                {
                    if (func.get_min_int_digits() == 0)
                        m_current_style.code += "#";

                    for (size_t i = 0; i < func.get_min_int_digits(); i++)
                    {
                        m_current_style.code += "0";
                    }
                }
                if (func.has_decimal_places())
                {
                    m_current_style.code += ".";
                    for(size_t i = 0; i < func.get_decimal_places() ; i++)
                        m_current_style.code += "0";
                }
                break;
            }
            case XML_currency_style:
            {
                generic_style_attr_parser func;
                func = std::for_each(attrs.begin(), attrs.end(), func);
                m_current_style.name = func.get_style_name();
                m_current_style.is_volatile = func.is_volatile();
                break;
            }
            case XML_percentage_style:
            {
                generic_style_attr_parser func;
                func = std::for_each(attrs.begin(), attrs.end(), func);
                m_current_style.name = func.get_style_name();
                m_current_style.is_volatile = func.is_volatile();
                break;
            }
            case XML_scientific_number:
            {
                scientific_number_attr_parser func;
                func = std::for_each(attrs.begin(), attrs.end(), func);

                if (func.is_grouped())
                {
                    if (func.get_min_int_digits() < 4)
                    {
                        m_current_style.code += "#,";
                        for (size_t i = 0; i < 3 - func.get_min_int_digits(); i++)
                        {
                            m_current_style.code += "#";
                        }
                        for (size_t i = 0; i < func.get_min_int_digits(); i++)
                        {
                            m_current_style.code += "0";
                        }
                    }
                    else
                    {
                        std::string temporary_code;
                        for(size_t i = 0; i < func.get_min_int_digits(); i++)
                        {
                            if (i % 3 == 0 && i != 0)
                                temporary_code += ",";
                            temporary_code += "0";
                        }
                        std::reverse(temporary_code.begin(), temporary_code.end());
                        m_current_style.code += temporary_code;
                    }
                }
                else
                {
                    if (func.get_min_int_digits() == 0)
                        m_current_style.code += "#";

                    for (size_t i = 0; i < func.get_min_int_digits(); i++)
                    {
                        m_current_style.code += "0";
                    }
                }

                m_current_style.code += ".";
                for(size_t i = 0; i < func.get_decimal_places() ; i++)
                    m_current_style.code += "0";

                m_current_style.code += "E+";
                for(size_t i = 0; i < func.get_min_exp_digits() ; i++)
                    m_current_style.code += "0";
                break;
            }
            case XML_boolean_style:
            {
                generic_style_attr_parser func;
                func = std::for_each(attrs.begin(), attrs.end(), func);
                m_current_style.name = func.get_style_name();
                m_current_style.is_volatile = func.is_volatile();
                break;
            }
            case XML_boolean:
            {
                m_current_style.code += "BOOLEAN";
                break;
            }
            case XML_fraction:
            {
                fraction_attr_parser func;
                func = std::for_each(attrs.begin(), attrs.end(), func);

                for (size_t i = 0; i < func.get_min_int_digits(); i++)
                    m_current_style.code += "#";

                if (func.get_min_int_digits() != 0)
                    m_current_style.code += " ";

                for (size_t i = 0; i < func.get_min_num_digits(); i++)
                    m_current_style.code += "?";

                m_current_style.code += "/";
                if (func.has_predefined_deno())
                    m_current_style.code += func.get_deno_value();
                else
                    for(size_t i = 0; i < func.get_min_deno_digits(); i++)
                        m_current_style.code += "?";
                break;
            }
            case XML_date_style:
            {
                generic_style_attr_parser func;
                func = std::for_each(attrs.begin(), attrs.end(), func);
                m_current_style.name = func.get_style_name();
                m_current_style.is_volatile = func.is_volatile();
                break;
            }
            case XML_day:
            {
                generic_style_attr_parser func;
                func = std::for_each(attrs.begin(), attrs.end(), func);
                m_current_style.code += "D";
                if (func.has_long())
                    m_current_style.code += "D";
                break;
            }
            case XML_month:
            {
                month_attr_parser func;
                func = std::for_each(attrs.begin(), attrs.end(), func);
                m_current_style.code += "M";
                if (func.has_long())
                    m_current_style.code += "M";
                if (func.is_textual())
                    m_current_style.code += "M";
                if (func.has_long() && func.is_textual())
                    m_current_style.code += "M";
                break;
            }
            case XML_year:
            {
                generic_style_attr_parser func;
                func = std::for_each(attrs.begin(), attrs.end(), func);
                m_current_style.code += "YY";
                if (func.has_long())
                    m_current_style.code += "YY";
                break;
            }
            case XML_time_style:
            {
                generic_style_attr_parser func;
                func = std::for_each(attrs.begin(), attrs.end(), func);
                m_current_style.name = func.get_style_name();
                m_current_style.is_volatile = func.is_volatile();
                break;
            }
            case XML_hours:
            {
                generic_style_attr_parser func;
                func = std::for_each(attrs.begin(), attrs.end(), func);
                m_current_style.code += "H";
                if (func.has_long())
                    m_current_style.code += "H";
                break;
            }
            case XML_minutes:
            {
                generic_style_attr_parser func;
                func = std::for_each(attrs.begin(), attrs.end(), func);
                m_current_style.code += "M";
                if (func.has_long())
                    m_current_style.code += "M";
                break;
            }
            case XML_seconds:
            {
                seconds_attr_parser func;
                func = std::for_each(attrs.begin(), attrs.end(), func);
                m_current_style.code += "S";
                if (func.has_long())
                    m_current_style.code += "S";
                if (func.has_decimal_places())
                    for (size_t i = 0; i < func.get_decimal_places(); i++)
                        m_current_style.code += "S";
                break;
            }
            case XML_am_pm:
            {
                m_current_style.code += " AM/PM";
                break;
            }
            case XML_text_style:
            {
                generic_style_attr_parser func;
                func = std::for_each(attrs.begin(), attrs.end(), func);
                m_current_style.name = func.get_style_name();
                m_current_style.is_volatile = func.is_volatile();
                break;
            }
            case XML_text_content:
            {
                m_current_style.code += "@";
                break;
            }
            default:
                ;
        }
    }
    else if (ns == NS_odf_style)
    {
        switch (name)
        {
            case XML_text_properties:
            {
                text_properties_attr_parser func;
                func = std::for_each(attrs.begin(), attrs.end(), func);
                if (func.has_color())
                {
                    std::ostringstream os;
                    os << m_current_style.code << '[' << func.get_color() << ']';
                    m_current_style.code = os.str();
                }
                break;
            }
            case XML_map:
            {
                map_attr_parser func;
                func = std::for_each(attrs.begin(), attrs.end(), func);
                if (func.has_map())
                {
                    std::ostringstream os;
                    os << '[' << func.get_sign() << func.get_value() << ']' << m_current_style.code;
                    m_current_style.code = os.str();
                }
                break;
            }
            default:
                ;
        }
    }
}

bool number_format_context::end_element(xmlns_id_t ns, xml_token_t name)
{
    if (!mp_styles)
        return pop_stack(ns, name);

    auto* number_format = mp_styles->get_number_format();
    ENSURE_INTERFACE(number_format, import_number_format);

    std::string_view character_content = m_character_stream;

    if (ns == NS_odf_number)
    {
        switch (name)
        {
            case XML_number_style:
            case XML_currency_style:
            case XML_percentage_style:
            case XML_text_style:
            case XML_boolean_style:
            case XML_date_style:
            case XML_time_style:
            {
                if (m_current_style.is_volatile)
                {
                    m_current_style.code += ";";
                }
                else
                {
                    size_t id_number_format = 0;

                    if (!m_current_style.code.empty())
                    {
                        number_format->set_code(m_current_style.code);
                        id_number_format = number_format->commit();
                    }

                    auto* xf = mp_styles->get_xf(ss::xf_category_t::cell_style);
                    ENSURE_INTERFACE(xf, import_xf);

                    xf->set_number_format(id_number_format);

                    auto* cell_style = mp_styles->get_cell_style();
                    ENSURE_INTERFACE(cell_style, import_cell_style);

                    cell_style->set_name(m_current_style.name);
                    cell_style->set_xf(xf->commit());
                    cell_style->commit();
                    return true; // TODO: fix this
    //              return pop_stack(ns, name);
                }
                break;
            }
            case XML_currency_symbol:
            {
                std::ostringstream os;
                os << m_current_style.code << "[$" << character_content << ']';
                m_current_style.code = os.str();
                break;
            }
            case XML_text:
            {
                m_current_style.code += character_content;
                break;
            }
        }
    }

    return false; // TODO: fix this
//  return pop_stack(ns, name);
}


void number_format_context::characters(std::string_view str, bool transient)
{
    if (str != "\n")
    {
        if (transient)
            m_character_stream = m_pool.intern(str).first;
        else
            m_character_stream = str;
    }
}

void number_format_context::reset()
{
    m_current_style = odf_number_format{};
}

} // namespace orcus

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
