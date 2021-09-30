/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "orcus/css_selector.hpp"

namespace orcus {

css_simple_selector_t::css_simple_selector_t() :
    pseudo_classes(0) {}

void css_simple_selector_t::clear()
{
    name = std::string_view{};
    id = std::string_view{};
    classes.clear();
    pseudo_classes = 0;
}

bool css_simple_selector_t::empty() const
{
    return name.empty() && id.empty() && classes.empty() && !pseudo_classes;
}

bool css_simple_selector_t::operator== (const css_simple_selector_t& r) const
{
    if (name != r.name)
        return false;

    if (id != r.id)
        return false;

    if (classes != r.classes)
        return false;

    return pseudo_classes == r.pseudo_classes;
}

bool css_simple_selector_t::operator!= (const css_simple_selector_t& r) const
{
    return !operator==(r);
}

size_t css_simple_selector_t::hash::operator() (const css_simple_selector_t& ss) const
{
    std::hash<std::string_view> h;

    size_t val = h(ss.name);
    val += h(ss.id);

    for (std::string_view s : ss.classes)
        val += h(s);

    val += ss.pseudo_classes;

    return val;
}

bool css_chained_simple_selector_t::operator== (const css_chained_simple_selector_t& r) const
{
    return combinator == r.combinator && simple_selector == r.simple_selector;
}

css_chained_simple_selector_t::css_chained_simple_selector_t() :
    combinator(css::combinator_t::descendant) {}

css_chained_simple_selector_t::css_chained_simple_selector_t(const css_simple_selector_t& ss) :
    combinator(css::combinator_t::descendant), simple_selector(ss) {}

css_chained_simple_selector_t::css_chained_simple_selector_t(
    css::combinator_t op, const css_simple_selector_t& ss) :
    combinator(op), simple_selector(ss) {}

void css_selector_t::clear()
{
    first.clear();
    chained.clear();
}

bool css_selector_t::operator== (const css_selector_t& r) const
{
    return first == r.first && chained == r.chained;
}

css_property_value_t::css_property_value_t() :
    type(css::property_value_t::none), value{std::string_view{}} {}

css_property_value_t::css_property_value_t(const css_property_value_t& r) :
    type(r.type), value(r.value)
{
}

css_property_value_t::css_property_value_t(std::string_view _str) :
    type(css::property_value_t::string), value(_str) {}

css_property_value_t& css_property_value_t::operator= (const css_property_value_t& r)
{
    type = r.type;
    value = r.value;
    return *this;
}

void css_property_value_t::swap(css_property_value_t& r)
{
    std::swap(type, r.type);
    std::swap(value, r.value);
}

std::ostream& operator<< (std::ostream& os, const css_simple_selector_t& v)
{
    os << v.name;
    css_simple_selector_t::classes_type::const_iterator it = v.classes.begin(), ite = v.classes.end();
    for (; it != ite; ++it)
        os << '.' << *it;
    if (!v.id.empty())
        os << '#' << v.id;
    if (v.pseudo_classes)
        os << css::pseudo_class_to_string(v.pseudo_classes);
    return os;
}

std::ostream& operator<< (std::ostream& os, const css_selector_t& v)
{
    os << v.first;
    css_selector_t::chained_type::const_iterator it = v.chained.begin(), ite = v.chained.end();
    for (; it != ite; ++it)
    {
        const css_chained_simple_selector_t& css = *it;
        os << ' ';
        switch (css.combinator)
        {
            case css::combinator_t::direct_child:
                os << "> ";
            break;
            case css::combinator_t::next_sibling:
                os << "+ ";
            break;
            case css::combinator_t::descendant:
            default:
                ;
        }
        os << css.simple_selector;
    }
    return os;
}

std::ostream& operator<< (std::ostream& os, const css_property_value_t& v)
{
    const char* sep = ",";

    switch (v.type)
    {
        case css::property_value_t::hsl:
        {
            auto c = std::get<css::hsla_color_t>(v.value);
            os << "hsl("
                << (int)c.hue << sep
                << (int)c.saturation << sep
                << (int)c.lightness
                << ")";
            break;
        }
        case css::property_value_t::hsla:
        {
            auto c = std::get<css::hsla_color_t>(v.value);
            os << "hsla("
                << (int)c.hue << sep
                << (int)c.saturation << sep
                << (int)c.lightness << sep
                << c.alpha
                << ")";
            break;
        }
        case css::property_value_t::rgb:
        {
            auto c = std::get<css::rgba_color_t>(v.value);
            os << "rgb("
                << (int)c.red << sep
                << (int)c.green << sep
                << (int)c.blue
                << ")";
            break;
        }
        case css::property_value_t::rgba:
        {
            auto c = std::get<css::rgba_color_t>(v.value);
            os << "rgba("
                << (int)c.red << sep
                << (int)c.green << sep
                << (int)c.blue << sep
                << c.alpha
                << ")";
            break;
        }
        case css::property_value_t::string:
            os << std::get<std::string_view>(v.value);
            break;
        case css::property_value_t::url:
            os << "url(" << std::get<std::string_view>(v.value) << ")";
            break;
        case css::property_value_t::none:
        default:
            ;
    }

    return os;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
