/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_ORCUS_CSS_TYPES_HPP
#define INCLUDED_ORCUS_CSS_TYPES_HPP

#include "env.hpp"

#include <cstdlib>
#include <cstdint>
#include <string>

namespace orcus { namespace css {

enum class combinator_t
{
    /// 'E F' where F is a descendant of E.
    descendant,
    /// 'E > F' where F is a direct child of E.
    direct_child,
    /// 'E + F' where F is a direct sibling of E where E precedes F.
    next_sibling
};

/**
 * List of functions used as property values.
 */
enum class property_function_t
{
    unknown = 0,
    hsl,
    hsla,
    rgb,
    rgba,
    url
};

enum class property_value_t
{
    none = 0,
    string,
    hsl,
    hsla,
    rgb,
    rgba,
    url
};

struct rgba_color_t
{
    uint8_t red;   /// 0 to 255
    uint8_t green; /// 0 to 255
    uint8_t blue;  /// 0 to 255
    double alpha;
};

struct hsla_color_t
{
    uint8_t hue;        /// 0 to 255
    uint8_t saturation; /// 0 to 255
    uint8_t lightness;  /// 0 to 255
    double alpha;
};

using pseudo_element_t = uint16_t;
using pseudo_class_t = uint64_t;

ORCUS_PSR_DLLPUBLIC extern const pseudo_element_t pseudo_element_after;
ORCUS_PSR_DLLPUBLIC extern const pseudo_element_t pseudo_element_before;
ORCUS_PSR_DLLPUBLIC extern const pseudo_element_t pseudo_element_first_letter;
ORCUS_PSR_DLLPUBLIC extern const pseudo_element_t pseudo_element_first_line;
ORCUS_PSR_DLLPUBLIC extern const pseudo_element_t pseudo_element_selection;
ORCUS_PSR_DLLPUBLIC extern const pseudo_element_t pseudo_element_backdrop;

ORCUS_PSR_DLLPUBLIC extern const pseudo_class_t pseudo_class_active;
ORCUS_PSR_DLLPUBLIC extern const pseudo_class_t pseudo_class_checked;
ORCUS_PSR_DLLPUBLIC extern const pseudo_class_t pseudo_class_default;
ORCUS_PSR_DLLPUBLIC extern const pseudo_class_t pseudo_class_dir;
ORCUS_PSR_DLLPUBLIC extern const pseudo_class_t pseudo_class_disabled;
ORCUS_PSR_DLLPUBLIC extern const pseudo_class_t pseudo_class_empty;
ORCUS_PSR_DLLPUBLIC extern const pseudo_class_t pseudo_class_enabled;
ORCUS_PSR_DLLPUBLIC extern const pseudo_class_t pseudo_class_first;
ORCUS_PSR_DLLPUBLIC extern const pseudo_class_t pseudo_class_first_child;
ORCUS_PSR_DLLPUBLIC extern const pseudo_class_t pseudo_class_first_of_type;
ORCUS_PSR_DLLPUBLIC extern const pseudo_class_t pseudo_class_fullscreen;
ORCUS_PSR_DLLPUBLIC extern const pseudo_class_t pseudo_class_focus;
ORCUS_PSR_DLLPUBLIC extern const pseudo_class_t pseudo_class_hover;
ORCUS_PSR_DLLPUBLIC extern const pseudo_class_t pseudo_class_indeterminate;
ORCUS_PSR_DLLPUBLIC extern const pseudo_class_t pseudo_class_in_range;
ORCUS_PSR_DLLPUBLIC extern const pseudo_class_t pseudo_class_invalid;
ORCUS_PSR_DLLPUBLIC extern const pseudo_class_t pseudo_class_lang;
ORCUS_PSR_DLLPUBLIC extern const pseudo_class_t pseudo_class_last_child;
ORCUS_PSR_DLLPUBLIC extern const pseudo_class_t pseudo_class_last_of_type;
ORCUS_PSR_DLLPUBLIC extern const pseudo_class_t pseudo_class_left;
ORCUS_PSR_DLLPUBLIC extern const pseudo_class_t pseudo_class_link;
ORCUS_PSR_DLLPUBLIC extern const pseudo_class_t pseudo_class_not;
ORCUS_PSR_DLLPUBLIC extern const pseudo_class_t pseudo_class_nth_child;
ORCUS_PSR_DLLPUBLIC extern const pseudo_class_t pseudo_class_nth_last_child;
ORCUS_PSR_DLLPUBLIC extern const pseudo_class_t pseudo_class_nth_last_of_type;
ORCUS_PSR_DLLPUBLIC extern const pseudo_class_t pseudo_class_nth_of_type;
ORCUS_PSR_DLLPUBLIC extern const pseudo_class_t pseudo_class_only_child;
ORCUS_PSR_DLLPUBLIC extern const pseudo_class_t pseudo_class_only_of_type;
ORCUS_PSR_DLLPUBLIC extern const pseudo_class_t pseudo_class_optional;
ORCUS_PSR_DLLPUBLIC extern const pseudo_class_t pseudo_class_out_of_range;
ORCUS_PSR_DLLPUBLIC extern const pseudo_class_t pseudo_class_read_only;
ORCUS_PSR_DLLPUBLIC extern const pseudo_class_t pseudo_class_read_write;
ORCUS_PSR_DLLPUBLIC extern const pseudo_class_t pseudo_class_required;
ORCUS_PSR_DLLPUBLIC extern const pseudo_class_t pseudo_class_right;
ORCUS_PSR_DLLPUBLIC extern const pseudo_class_t pseudo_class_root;
ORCUS_PSR_DLLPUBLIC extern const pseudo_class_t pseudo_class_scope;
ORCUS_PSR_DLLPUBLIC extern const pseudo_class_t pseudo_class_target;
ORCUS_PSR_DLLPUBLIC extern const pseudo_class_t pseudo_class_valid;
ORCUS_PSR_DLLPUBLIC extern const pseudo_class_t pseudo_class_visited;

/**
 * Convert a textural representation of a pseudo element into its numerical
 * representation.
 */
ORCUS_PSR_DLLPUBLIC pseudo_element_t to_pseudo_element(std::string_view s);

/**
 * Convert a textural representation of a pseudo class into its numerical
 * representation.
 */
ORCUS_PSR_DLLPUBLIC pseudo_class_t to_pseudo_class(std::string_view s);

ORCUS_PSR_DLLPUBLIC std::string pseudo_class_to_string(pseudo_class_t val);

ORCUS_PSR_DLLPUBLIC property_function_t to_property_function(std::string_view s);

}}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
