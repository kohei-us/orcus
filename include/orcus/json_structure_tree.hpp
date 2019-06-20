/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_ORCUS_JSON_STRUCTURE_TREE_HPP
#define INCLUDED_ORCUS_JSON_STRUCTURE_TREE_HPP

#include "orcus/env.hpp"
#include "orcus/types.hpp"

#include <ostream>
#include <memory>

namespace orcus { namespace json {

class ORCUS_DLLPUBLIC structure_tree
{
    struct impl;
    std::unique_ptr<impl> mp_impl;

public:
    structure_tree(const structure_tree&) = delete;
    structure_tree& operator= (const structure_tree&) = delete;

    structure_tree();
    ~structure_tree();

    void parse(const char* p, size_t n);

    void dump_compact(std::ostream& os) const;
};

}}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
