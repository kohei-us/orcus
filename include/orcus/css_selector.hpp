/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_ORCUS_CSS_SELECTOR_HPP
#define INCLUDED_ORCUS_CSS_SELECTOR_HPP

#include "env.hpp"
#include "css_types.hpp"

#include <ostream>
#include <variant>
#include <vector>
#include <unordered_set>
#include <unordered_map>

namespace orcus {

struct ORCUS_DLLPUBLIC css_simple_selector_t
{
    typedef std::unordered_set<std::string_view> classes_type;

    std::string_view name;
    std::string_view id;
    classes_type classes;
    css::pseudo_class_t pseudo_classes;

    css_simple_selector_t();

    void clear();
    bool empty() const;

    bool operator== (const css_simple_selector_t& r) const;
    bool operator!= (const css_simple_selector_t& r) const;

    struct hash
    {
        size_t operator() (const css_simple_selector_t& ss) const;
    };
};

struct ORCUS_DLLPUBLIC css_chained_simple_selector_t
{
    css::combinator_t combinator;
    css_simple_selector_t simple_selector;

    bool operator== (const css_chained_simple_selector_t& r) const;

    css_chained_simple_selector_t();
    css_chained_simple_selector_t(const css_simple_selector_t& ss);
    css_chained_simple_selector_t(css::combinator_t op, const css_simple_selector_t& ss);
};

/**
 * Each CSS selector consists of one or more chained simple selectors.
 */
struct ORCUS_DLLPUBLIC css_selector_t
{
    typedef std::vector<css_chained_simple_selector_t> chained_type;
    css_simple_selector_t first;
    chained_type chained;

    void clear();

    bool operator== (const css_selector_t& r) const;
};

/**
 * Structure representing a single CSS property value.
 */
struct ORCUS_DLLPUBLIC css_property_value_t
{
    using value_type = std::variant<std::string_view, css::rgba_color_t, css::hsla_color_t>;

    css::property_value_t type;
    value_type value;

    css_property_value_t();
    css_property_value_t(const css_property_value_t& r);

    /**
     * Constructor that takes a string value.
     *
     * @param _str string value to store. This value should point to a string
     *            buffer that's already been interned. The caller is
     *            responsible for managing the life cycle of the source string
     *            buffer.
     */
    css_property_value_t(std::string_view _str);

    css_property_value_t& operator= (const css_property_value_t& r);

    void swap(css_property_value_t& r);
};

typedef std::unordered_map<std::string_view, std::vector<css_property_value_t>> css_properties_t;
typedef std::unordered_map<css::pseudo_element_t, css_properties_t> css_pseudo_element_properties_t;

ORCUS_DLLPUBLIC std::ostream& operator<< (std::ostream& os, const css_simple_selector_t& v);
ORCUS_DLLPUBLIC std::ostream& operator<< (std::ostream& os, const css_selector_t& v);
ORCUS_DLLPUBLIC std::ostream& operator<< (std::ostream& os, const css_property_value_t& v);

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
