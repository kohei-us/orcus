/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_ORCUS_CSS_DOCUMENT_TREE_HPP
#define INCLUDED_ORCUS_CSS_DOCUMENT_TREE_HPP

#include "orcus/css_selector.hpp"
#include "orcus/exception.hpp"

#include <string>
#include <memory>

namespace orcus {

/**
 * Class representing CSS rules.
 */
class ORCUS_DLLPUBLIC css_document_tree
{
    struct impl;
    std::unique_ptr<impl> mp_impl;

public:

    class insertion_error : public general_error
    {
    public:
        insertion_error(const std::string& msg);
    };

    css_document_tree(const css_document_tree&) = delete;

    css_document_tree();
    css_document_tree(css_document_tree&& other);
    ~css_document_tree();

    css_document_tree& operator=(css_document_tree&& other);

    /**
     * Load raw string stream containing CSS rules to populate the document
     * tree.
     *
     * @param stream raw CSS rules.
     */
    void load(std::string_view stream);

    /**
     * Insert or replace properties for given selector and pseudo element
     * flags.
     *
     * @param selector selector to store properties for.
     * @param pseudo_elem pseudo element flags for the last simple selector.
     * @param props new properties to insert.
     */
    void insert_properties(
        const css_selector_t& selector,
        css::pseudo_element_t pseudo_elem,
        const css_properties_t& props);

    /**
     * Get properties associated with given selector and one or more pseudo
     * elements.
     *
     * @param selector selector to get properties for.
     * @param pseudo_elem pseudo element flags for the last simple selector.
     *                    This value is a bitfield.
     *
     * @return const pointer to the property set instance, or NULL in case
     *         there is no properties for the given selector.
     */
    const css_properties_t* get_properties(
        const css_selector_t& selector, css::pseudo_element_t pseudo_elem) const;

    /**
     * Get all sets of properties associated with given selector, for all
     * pseudo element values.
     *
     * @param selector selector to get properties for.
     *
     * @return const pointer to the map of property sets with pseudo element
     *         values as the keys, or NULL in case there is no properties for
     *         the given selector.
     */
    const css_pseudo_element_properties_t*
        get_all_properties(const css_selector_t& selector) const;

    void dump() const;

    void swap(css_document_tree& other) noexcept;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
