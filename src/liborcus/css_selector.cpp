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
    type(css::property_value_t::none), str(nullptr) {}

namespace
{

void assign_value(css_property_value_t& v, const css_property_value_t& r)
{
    switch (r.type)
    {
        case css::property_value_t::rgb:
        case css::property_value_t::rgba:
            v.red = r.red;
            v.green = r.green;
            v.blue = r.blue;
            v.alpha = r.alpha;
        break;
        case css::property_value_t::hsl:
        case css::property_value_t::hsla:
            v.hue = r.hue;
            v.saturation = r.saturation;
            v.lightness = r.lightness;
            v.alpha = r.alpha;
        break;
        case css::property_value_t::string:
        case css::property_value_t::url:
            v.str = r.str;
            v.length = r.length;
        break;
        case css::property_value_t::none:
        default:
            ;
    }
}

}

css_property_value_t::css_property_value_t(const css_property_value_t& r) :
    type(r.type)
{
    assign_value(*this, r);
}

css_property_value_t::css_property_value_t(std::string_view _str) :
    type(css::property_value_t::string), str(_str.data()), length(_str.size()) {}

css_property_value_t& css_property_value_t::operator= (const css_property_value_t& r)
{
    if (&r != this)
    {
        type = r.type;
        assign_value(*this, r);
    }
    return *this;
}

void css_property_value_t::swap(css_property_value_t& r)
{
    if (&r != this)
    {
        css_property_value_t tmp(*this);
        *this = r;
        r = tmp;
    }
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
            os << "hsl("
               << (int)v.hue << sep
               << (int)v.saturation << sep
               << (int)v.lightness
               << ")";
        break;
        case css::property_value_t::hsla:
            os << "hsla("
               << (int)v.hue << sep
               << (int)v.saturation << sep
               << (int)v.lightness << sep
               << v.alpha
               << ")";
        break;
        case css::property_value_t::rgb:
            os << "rgb("
               << (int)v.red << sep
               << (int)v.green << sep
               << (int)v.blue
               << ")";
        break;
        case css::property_value_t::rgba:
            os << "rgba("
               << (int)v.red << sep
               << (int)v.green << sep
               << (int)v.blue << sep
               << v.alpha
               << ")";
        break;
        case css::property_value_t::string:
            os << std::string_view(v.str, v.length);
        break;
        case css::property_value_t::url:
            os << "url(" << std::string_view(v.str, v.length) << ")";
        break;
        case css::property_value_t::none:
        default:
            ;
    }

    return os;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
