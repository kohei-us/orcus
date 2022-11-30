/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_ORCUS_CSS_PARSER_HPP
#define INCLUDED_ORCUS_CSS_PARSER_HPP

#define ORCUS_DEBUG_CSS 0

#include "parser_global.hpp"
#include "css_parser_base.hpp"

#include <cassert>
#include <algorithm>

#if ORCUS_DEBUG_CSS
#include <iostream>
using std::cout;
using std::endl;
#endif

namespace orcus {

/**
 * Empty handler for CSS parser.  Sub-class from it and implement necessary
 * methods.
 */
class css_handler
{
public:
    void at_rule_name(std::string_view name)
    {
        (void)name;
    }

    void simple_selector_type(std::string_view type)
    {
        (void)type;
    }

    void simple_selector_class(std::string_view cls)
    {
        (void)cls;
    }

    void simple_selector_pseudo_element(orcus::css::pseudo_element_t pe)
    {
        (void)pe;
    }

    void simple_selector_pseudo_class(orcus::css::pseudo_class_t pc)
    {
        (void)pc;
    }

    void simple_selector_id(std::string_view id)
    {
        (void)id;
    }

    void end_simple_selector() {}

    void end_selector() {}

    void combinator(orcus::css::combinator_t combinator)
    {
        (void)combinator;
    }

    /**
     * Called at each property name.
     *
     * @param name property name string.
     */
    void property_name(std::string_view name)
    {
        (void)name;
    }

    /**
     * Called at each ordinary property value string.
     *
     * @param value value string.
     */
    void value(std::string_view value)
    {
        (void)value;
    }

    /**
     * Called at each RGB color value of a property.
     *
     * @param red value of red (0-255)
     * @param green value of green (0-255)
     * @param blue value of blue (0-255)
     */
    void rgb(uint8_t red, uint8_t green, uint8_t blue)
    {
        (void)red; (void)green; (void)blue;
    }

    /**
     * Called at each RGB color value of a property with alpha transparency
     * value.
     *
     * @param red value of red (0-255)
     * @param green value of green (0-255)
     * @param blue value of blue (0-255)
     * @param alpha alpha transparency value
     */
    void rgba(uint8_t red, uint8_t green, uint8_t blue, double alpha)
    {
        (void)red; (void)green; (void)blue; (void)alpha;
    }

    /**
     * Called at each HSL color value of a property.
     *
     * @param hue hue
     * @param sat saturation
     * @param light lightness
     */
    void hsl(uint8_t hue, uint8_t sat, uint8_t light)
    {
        (void)hue; (void)sat; (void)light;
    }

    /**
     * Called at each HSL color value of a property with alpha transparency
     * value.
     *
     * @param hue hue
     * @param sat saturation
     * @param light lightness
     * @param alpha alpha value
     */
    void hsla(uint8_t hue, uint8_t sat, uint8_t light, double alpha)
    {
        (void)hue; (void)sat; (void)light; (void)alpha;
    }

    /**
     * Called at each URL value of a property.
     *
     * @param url URL value string.
     */
    void url(std::string_view url)
    {
        (void)url;
    }

    /**
     * Called when the parsing begins.
     */
    void begin_parse() {}

    /**
     * Called when the parsing ends.
     */
    void end_parse() {}

    /**
     * Called at the beginning of each block.  An opening brace '{' marks the
     * beginning of a block.
     */
    void begin_block() {}

    /**
     * Called at the end of each block.  A closing brace '}' marks the end of
     * a block.
     */
    void end_block() {}

    /**
     * Called at the beginning of each property.
     */
    void begin_property() {}

    /**
     * Called at the end of each property.
     */
    void end_property() {}
};

/**
 * Parser for CSS documents.
 *
 * @tparam HandlerT Hanlder type with member functions for event callbacks.
 *         Refer to css_handler.
 */
template<typename HandlerT>
class css_parser : public css::parser_base
{
public:
    typedef HandlerT handler_type;

    css_parser(std::string_view content, handler_type& hdl);
    void parse();

private:
    // Handlers - at the time a handler is called the current position is
    // expected to point to the first unprocessed non-blank character, and
    // each handler must set the current position to the next unprocessed
    // non-blank character when it finishes.
    void rule();
    void at_rule_name();
    void simple_selector_name();
    void property_name();
    void property();
    void quoted_value(char c);
    void value();
    void function_value(std::string_view v);
    void function_rgb(bool alpha);
    void function_hsl(bool alpha);
    void function_url();
    void name_sep();
    void property_sep();
    void block();

    handler_type& m_handler;
};

template<typename _Handler>
css_parser<_Handler>::css_parser(std::string_view content, handler_type& hdl) :
    css::parser_base(content), m_handler(hdl) {}

template<typename _Handler>
void css_parser<_Handler>::parse()
{
    shrink_stream();

#if ORCUS_DEBUG_CSS
    std::cout << "compressed: '";
    const char* p = mp_char;
    for (; p != mp_end; ++p)
        std::cout << *p;
    std::cout << "'" << std::endl;
#endif
    m_handler.begin_parse();
    while (has_char())
        rule();
    m_handler.end_parse();
}

template<typename _Handler>
void css_parser<_Handler>::rule()
{
    // <selector name> , ... , <selector name> <block>
    while (has_char())
    {
        if (skip_comment())
            continue;

        char c = cur_char();
        if (is_alpha(c))
        {
            simple_selector_name();
            continue;
        }

        switch (c)
        {
            case '>':
                set_combinator(c, css::combinator_t::direct_child);
            break;
            case '+':
                set_combinator(c, css::combinator_t::next_sibling);
            break;
            case '.':
            case '#':
            case '@':
                simple_selector_name();
            break;
            case ',':
                name_sep();
            break;
            case '{':
                reset_before_block();
                block();
            break;
            default:
                parse_error::throw_with("rule: failed to parse '", c, "'", offset());
        }
    }
}

template<typename _Handler>
void css_parser<_Handler>::at_rule_name()
{
    assert(has_char());
    assert(cur_char() == '@');
    next();
    char c = cur_char();
    if (!is_alpha(c))
        throw parse_error("at_rule_name: first character of an at-rule name must be an alphabet.", offset());

    const char* p;
    size_t len;
    identifier(p, len);
    skip_blanks();

    m_handler.at_rule_name({p, len});
#if ORCUS_DEBUG_CSS
    std::string foo(p, len);
    std::cout << "at-rule name: " << foo.c_str() << std::endl;
#endif
}

template<typename _Handler>
void css_parser<_Handler>::simple_selector_name()
{
    assert(has_char());
    char c = cur_char();
    if (c == '@')
    {
        // This is the name of an at-rule.
        at_rule_name();
        return;
    }

    if (m_simple_selector_count)
    {
#if ORCUS_DEBUG_CSS
    cout << "combinator: " << m_combinator << endl;
#endif
        m_handler.combinator(m_combinator);
        m_combinator = css::combinator_t::descendant;
    }
    assert(is_alpha(c) || c == '.' || c == '#');

    const char* p = nullptr;
    size_t n = 0;

#if ORCUS_DEBUG_CSS
    cout << "simple_selector_name: (" << m_simple_selector_count << ")";
#endif

    if (c != '.' && c != '#')
    {
        identifier(p, n);
#if ORCUS_DEBUG_CSS
        std::string s(p, n);
        cout << " type=" << s;
#endif
        m_handler.simple_selector_type({p, n});
    }

    bool in_loop = true;
    while (in_loop && has_char())
    {
        switch (cur_char())
        {
            case '.':
            {
                next();
                identifier(p, n);
                m_handler.simple_selector_class({p, n});
#if ORCUS_DEBUG_CSS
                std::string s(p, n);
                std::cout << " class=" << s;
#endif
            }
            break;
            case '#':
            {
                next();
                identifier(p, n);
                m_handler.simple_selector_id({p, n});
#if ORCUS_DEBUG_CSS
                std::string s(p, n);
                std::cout << " id=" << s;
#endif
            }
            break;
            case ':':
            {
                // This could be either a pseudo element or pseudo class.
                next();
                if (cur_char() == ':')
                {
                    // pseudo element.
                    next();
                    identifier(p, n);
                    css::pseudo_element_t elem = css::to_pseudo_element({p, n});
                    if (!elem)
                        parse_error::throw_with(
                            "selector_name: unknown pseudo element '", {p, n}, "'", offset());

                    m_handler.simple_selector_pseudo_element(elem);
                }
                else
                {
                    // pseudo class (or pseudo element in the older version of CSS).
                    identifier(p, n);
                    css::pseudo_class_t pc = css::to_pseudo_class({p, n});
                    if (!pc)
                        parse_error::throw_with(
                            "selector_name: unknown pseudo class '", {p, n}, "'", offset());

                    m_handler.simple_selector_pseudo_class(pc);
                }
            }
            break;
            default:
                in_loop = false;
        }
    }

    m_handler.end_simple_selector();
    skip_comments_and_blanks();

    ++m_simple_selector_count;

#if ORCUS_DEBUG_CSS
    std::cout << std::endl;
#endif
}

template<typename _Handler>
void css_parser<_Handler>::property_name()
{
    // <identifier>

    assert(has_char());
    char c = cur_char();
    if (!is_alpha(c) && c != '.')
        parse_error::throw_with(
            "property_name: first character of a name must be an alphabet or a dot, but found '", c, "'", offset());

    const char* p;
    size_t len;
    identifier(p, len);
    skip_comments_and_blanks();

    m_handler.property_name({p, len});
#if ORCUS_DEBUG_CSS
    std::string foo(p, len);
    std::cout << "property name: " << foo.c_str() << std::endl;
#endif
}

template<typename _Handler>
void css_parser<_Handler>::property()
{
    // <property name> : <value> , ... , <value>

    m_handler.begin_property();
    property_name();
    if (cur_char() != ':')
        throw parse_error("property: ':' expected.", offset());
    next();
    skip_comments_and_blanks();

    bool in_loop = true;
    while (in_loop && has_char())
    {
        value();
        char c = cur_char();
        switch (c)
        {
            case ',':
            {
                // separated by commas.
                next();
                skip_comments_and_blanks();
            }
            break;
            case ';':
            case '}':
                in_loop = false;
            break;
            default:
                ;
        }
    }

    skip_comments_and_blanks();
    m_handler.end_property();
}

template<typename _Handler>
void css_parser<_Handler>::quoted_value(char c)
{
    // Parse until the the end quote is reached.
    const char* p = nullptr;
    size_t len = 0;
    literal(p, len, c);
    next();
    skip_blanks();

    m_handler.value({p, len});
#if ORCUS_DEBUG_CSS
    std::string foo(p, len);
    std::cout << "quoted value: " << foo.c_str() << std::endl;
#endif
}

template<typename _Handler>
void css_parser<_Handler>::value()
{
    assert(has_char());
    char c = cur_char();
    if (c == '"' || c == '\'')
    {
        quoted_value(c);
        return;
    }

    std::string_view v = parse_value();
    if (v.empty())
        return;

    if (cur_char() == '(')
    {
        function_value(v);
        return;
    }

    m_handler.value(v);

    skip_comments_and_blanks();

#if ORCUS_DEBUG_CSS
    std::cout << "value: " << v << std::endl;
#endif
}

template<typename _Handler>
void css_parser<_Handler>::function_value(std::string_view v)
{
    assert(cur_char() == '(');
    css::property_function_t func = css::to_property_function(v);
    if (func == css::property_function_t::unknown)
        parse_error::throw_with("function_value: unknown function '", v, "'", offset());

    // Move to the first character of the first argument.
    next();
    skip_comments_and_blanks();

    switch (func)
    {
        case css::property_function_t::rgb:
            function_rgb(false);
        break;
        case css::property_function_t::rgba:
            function_rgb(true);
        break;
        case css::property_function_t::hsl:
            function_hsl(false);
        break;
        case css::property_function_t::hsla:
            function_hsl(true);
        break;
        case css::property_function_t::url:
            function_url();
        break;
        default:
            parse_error::throw_with("function_value: unhandled function '", v, "'", offset());
    }

    char c = cur_char();
    if (c != ')')
        parse_error::throw_with("function_value: ')' expected but '", c, "' found.", offset());

    next();
    skip_comments_and_blanks();
}

template<typename _Handler>
void css_parser<_Handler>::function_rgb(bool alpha)
{
    // rgb(num, num, num)  rgba(num, num, num, float)

    uint8_t vals[3];
    uint8_t* p = vals;
    const uint8_t* plast = p + 2;
    char c = 0;

    for (; ; ++p)
    {
        *p = parse_uint8();

        skip_comments_and_blanks();

        if (p == plast)
            break;

        c = cur_char();

        if (c != ',')
            parse_error::throw_with("function_rgb: ',' expected but '", c, "' found.", offset());

        next();
        skip_comments_and_blanks();
    }

    if (alpha)
    {
        c = cur_char();
        if (c != ',')
            parse_error::throw_with("function_rgb: ',' expected but '", c, "' found.", offset());

        next();
        skip_comments_and_blanks();

        double alpha_val = parse_double_or_throw();

        alpha_val = std::clamp(alpha_val, 0.0, 1.0);
        m_handler.rgba(vals[0], vals[1], vals[2], alpha_val);
    }
    else
        m_handler.rgb(vals[0], vals[1], vals[2]);

#if ORCUS_DEBUG_CSS
    std::cout << "rgb";
    if (alpha)
        std::cout << 'a';
    std::cout << '(';
    p = vals;
    const uint8_t* pend = plast + 1;
    for (; p != pend; ++p)
        std::cout << ' ' << (int)*p;
    std::cout << " )" << std::endl;
#endif
}

template<typename _Handler>
void css_parser<_Handler>::function_hsl(bool alpha)
{
    // hsl(num, percent, percent)  hsla(num, percent, percent, float)

    double hue = parse_double_or_throw(); // casted to uint8_t eventually.
    hue = std::clamp(hue, 0.0, 360.0);
    skip_comments_and_blanks();

    char c = cur_char();
    if (c != ',')
        parse_error::throw_with("function_hsl: ',' expected but '", c, "' found.", offset());

    next();
    skip_comments_and_blanks();

    double sat = parse_percent();
    sat = std::clamp(sat, 0.0, 100.0);
    skip_comments_and_blanks();

    c = cur_char();
    if (c != ',')
        parse_error::throw_with("function_hsl: ',' expected but '", c, "' found.", offset());

    next();
    skip_comments_and_blanks();

    double light = parse_percent();
    light = std::clamp(light, 0.0, 100.0);
    skip_comments_and_blanks();

    if (!alpha)
    {
        m_handler.hsl(hue, sat, light);
        return;
    }

    c = cur_char();
    if (c != ',')
        parse_error::throw_with("function_hsl: ',' expected but '", c, "' found.", offset());

    next();
    skip_comments_and_blanks();

    double alpha_val = parse_double_or_throw();
    alpha_val = std::clamp(alpha_val, 0.0, 1.0);
    skip_comments_and_blanks();
    m_handler.hsla(hue, sat, light, alpha_val);
}

template<typename _Handler>
void css_parser<_Handler>::function_url()
{
    char c = cur_char();

    if (c == '"' || c == '\'')
    {
        // Quoted URL value.
        const char* p;
        size_t len;
        literal(p, len, c);
        next();
        skip_comments_and_blanks();
        m_handler.url({p, len});
#if ORCUS_DEBUG_CSS
        std::cout << "url(" << std::string(p, len) << ")" << std::endl;
#endif
        return;
    }

    // Unquoted URL value.
    const char* p;
    size_t len;
    skip_to_or_blank(p, len, ")");
    skip_comments_and_blanks();
    m_handler.url({p, len});
#if ORCUS_DEBUG_CSS
    std::cout << "url(" << std::string(p, len) << ")" << std::endl;
#endif
}

template<typename _Handler>
void css_parser<_Handler>::name_sep()
{
    assert(cur_char() == ',');
#if ORCUS_DEBUG_CSS
    std::cout << "," << std::endl;
#endif
    next();
    skip_blanks();
    m_handler.end_selector();
}

template<typename _Handler>
void css_parser<_Handler>::property_sep()
{
#if ORCUS_DEBUG_CSS
    std::cout << ";" << std::endl;
#endif
    next();
    skip_comments_and_blanks();
}

template<typename _Handler>
void css_parser<_Handler>::block()
{
    // '{' <property> ';' ... ';' <property> ';'(optional) '}'

    assert(cur_char() == '{');
#if ORCUS_DEBUG_CSS
    std::cout << "{" << std::endl;
#endif
    m_handler.end_selector();
    m_handler.begin_block();

    next();
    skip_comments_and_blanks();

    // parse properties.
    while (has_char())
    {
        property();
        if (cur_char() != ';')
            break;
        property_sep();
        if (cur_char() == '}')
            // ';' after the last property.  This is optional but allowed.
            break;
    }

    if (cur_char() != '}')
        throw parse_error("block: '}' expected.", offset());

    m_handler.end_block();

    next();
    skip_comments_and_blanks();

#if ORCUS_DEBUG_CSS
    std::cout << "}" << std::endl;
#endif
}

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
