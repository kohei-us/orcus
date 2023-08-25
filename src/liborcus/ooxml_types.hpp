/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_ORCUS_OOXML_TYPES_HPP
#define INCLUDED_ORCUS_OOXML_TYPES_HPP

#include <iostream>
#include <memory>
#include <unordered_map>

namespace orcus {

typedef const char* content_type_t;
typedef const char* schema_t;

/**
 * Part name (first) and content type (second).
 */
typedef ::std::pair<std::string_view, content_type_t> xml_part_t;

/**
 * Single OPC relationship that corresponds with a Relationship element in
 * .rels parts.
 */
struct opc_rel_t
{
    std::string_view  rid;
    std::string_view  target;
    schema_t type;

    opc_rel_t() : type(nullptr) {}
    opc_rel_t(std::string_view _rid, const std::string_view& _target, schema_t _type) :
        rid(_rid), target(_target), type(_type) {}
};

/**
 * Used as a base struct only to allow storage of custom data associated
 * with a relationship.
 */
struct opc_rel_extra
{
    virtual ~opc_rel_extra() = 0;
};

struct opc_rel_extras_t
{
    typedef std::unordered_map<std::string_view, std::unique_ptr<opc_rel_extra>> map_type;

    /**
     * Key is a textual relation ID, while the value is an arbitrary data
     * associated with the relation ID.
     */
    map_type data;

    opc_rel_extras_t(opc_rel_extras_t&& other);

    opc_rel_extras_t(const opc_rel_extras_t&) = delete;
    opc_rel_extras_t& operator=(const opc_rel_extras_t&) = delete;

    opc_rel_extras_t();
    ~opc_rel_extras_t();

    void swap(opc_rel_extras_t& other);
};

}

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
