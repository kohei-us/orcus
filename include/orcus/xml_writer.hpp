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

/**
 * This class lets you produce XML contents from scratch.  It writes its
 * content to any object supporting the std::ostream interface.
 */
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

    xml_writer(const xml_writer&) = delete;
    xml_writer& operator= (const xml_writer&) = delete;

    xml_writer(xmlns_repository& ns_repo, std::ostream& os);
    xml_writer(xml_writer&& other);

    xml_writer& operator= (xml_writer&& other);

    /**
     * Destructor. Any remaining element(s) on the stack will get popped when
     * the destructor is called.
     */
    ~xml_writer();

    /**
     * Push a new element to the stack, and write an opening element to the
     * output stream.  It differs from the {@link push_element} method in that
     * the new element will be automatically popped when the returned object
     * goes out of scope.
     *
     * @param name name of the new element.
     *
     * @return scope object which automatically pops the element when it goes
     *         out of scope.
     */
    scope push_element_scope(const xml_name_t& name);

    /**
     * Push a new element to the stack, and write an opening element to the
     * output stream.
     *
     * @param name name of the element.
     */
    void push_element(const xml_name_t& name);

    /**
     * Add a namespace definition for the next element to be pushed.
     *
     * @param alias alias for the namespace.
     * @param value value of the namespace definition.
     *
     * @return ID for the namespace being added.
     */
    xmlns_id_t add_namespace(std::string_view alias, std::string_view value);

    /**
     * Add a new attribute for the next element to be pushed.
     *
     * @param name name of the attribute to be added.
     * @param value value of the attribute to be added.
     */
    void add_attribute(const xml_name_t& name, std::string_view value);

    /**
     * Add a content to the current element on the stack.  The content will be
     * properly encoded.
     *
     * @param content content to be added to the current element.
     */
    void add_content(std::string_view content);

    /**
     * Pop the current element from the stack, and write a closing element to
     * the output stream.
     *
     * @return the name of the element being popped.
     */
    xml_name_t pop_element();
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
