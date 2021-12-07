/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_ORCUS_GLOBAL_HPP
#define INCLUDED_ORCUS_GLOBAL_HPP

#include "types.hpp"
#include "env.hpp"

#include <memory>
#include <functional>

#define ORCUS_ASCII(literal) literal, sizeof(literal)-1
#define ORCUS_N_ELEMENTS(name) sizeof(name)/sizeof(name[0])

namespace orcus {

class tokens;

ORCUS_DLLPUBLIC void print_element(xmlns_id_t ns, xml_token_t name); // TODO:API: not used. remove?

/**
 * Print attributes to stdout for debugging purposes.
 */
ORCUS_DLLPUBLIC void print_attrs(const tokens& tokens, const xml_attrs_t& attrs); // TODO:API: internal to liborcus

/**
 * Parse the string representation of a date-time value, and convert it into
 * a set of numerical values.  A string representation allows either a date
 * only or a date and time value.  It does not allow a time only value; it
 * always expects to have a date element.
 *
 * date only: 2013-04-09
 * date and time: 2013-04-09T21:34:09.55
 *
 * @param str string representation of a date-time value.
 * @return converted date-time value consisting of a set of numeric values.
 */
ORCUS_DLLPUBLIC date_time_t to_date_time(std::string_view str); // TODO:API: internal to liborcus

/**
 * Function object for deleting objects that are stored in map container as
 * pointers.
 */
template<typename T>
struct map_object_deleter // TODO:API: remove it and use unique_ptr
{
    void operator() (typename T::value_type& v)
    {
        delete v.second;
    }
};

}

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
