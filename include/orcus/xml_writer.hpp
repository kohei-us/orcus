/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_ORCUS_XML_WRITER_HPP
#define INCLUDED_ORCUS_XML_WRITER_HPP

#include "orcus/types.hpp"

#include <memory>

namespace orcus {

class xmlns_repository;

class ORCUS_PSR_DLLPUBLIC xml_writer
{
    struct impl;
    std::unique_ptr<impl> mp_impl;

    void close_current_element();

public:
    class ORCUS_PSR_DLLPUBLIC scope
    {
        friend class xml_writer;

        struct impl;
        std::unique_ptr<impl> mp_impl;

        scope(xml_writer* parent, const xml_name_t& name);
    public:
        scope(const scope&) = delete;
        scope(scope&& other);
        ~scope();

        scope& operator= (scope&& other);
    };

    xml_writer(xmlns_repository& ns_repo, std::ostream& os);
    ~xml_writer();

    scope set_element_scope(const xml_name_t& name);

    void push_element(const xml_name_t& name);

    xmlns_id_t add_namespace(const pstring& alias, const pstring& value);

    void add_attribute(const xml_name_t& name, const pstring& value);

    void add_content(const pstring& content);

    xml_name_t pop_element();
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
