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

namespace orcus {

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
