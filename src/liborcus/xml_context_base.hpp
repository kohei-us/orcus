/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_ORCUS_XML_CONTEXT_BASE_HPP
#define INCLUDED_ORCUS_XML_CONTEXT_BASE_HPP

#include "xml_stream_handler.hpp"
#include "pstring.hpp"

namespace orcus {

struct session_context;
class tokens;
class xmlns_context;

class xml_context_base
{
public:
    xml_context_base(const xml_context_base&) = delete;
    xml_context_base& operator=(const xml_context_base&) = delete;

    xml_context_base(session_context& session_cxt, const tokens& tokens);
    virtual ~xml_context_base() = 0;

    virtual void declaration(const xml_declaration_t& decl);

    /**
     * Whether or not the current context object can handle the specified
     * element.  This method is used by the caller to determine whether to use
     * a child context to handle the specified element and its child elements.
     * If a child context needs to be used, the caller will call the
     * create_child_context method to obtain the child context object.
     *
     * @param ns namespace value for the element.
     * @param name name of the element.
     *
     * @return true if the current context object can handle this element,
     *         false if the caller needs to create a child context to handle
     *         the element.
     */
    virtual bool can_handle_element(xmlns_id_t ns, xml_token_t name) const = 0;

    /**
     * This method gets called by the stream handler to fetch a child context
     * object when the call to can_handle_element() returns false.  The caller
     * is not responsible for managing the life cycle of the returned context
     * object; the current context object must manage the life cycle of the
     * context object it returns.
     *
     * @param ns namespace value for the element.
     * @param name name of the element.
     *
     * @return pointer to the context object that should handle the element.
     */
    virtual xml_context_base* create_child_context(xmlns_id_t ns, xml_token_t name) = 0;

    /**
     * This method gets called when the child context is about to get phased
     * out, to give the parent context object to communicate with the child
     * context object before it gets phased out.
     *
     * @param ns namespace value for the element.
     * @param name name of the element.
     * @param child pointer to the child context object that is about to get
     *              phased out.
     */
    virtual void end_child_context(xmlns_id_t ns, xml_token_t name, xml_context_base* child) = 0;

    virtual void start_element(xmlns_id_t ns, xml_token_t name, const ::std::vector<xml_token_attr_t>& attrs) = 0;

    /**
     * Called on closing element.
     *
     * @param ns namespace token
     * @param name element name
     *
     * @return true if the base element of the context is closing, false
     *         otherwise.
     */
    virtual bool end_element(xmlns_id_t ns, xml_token_t name) = 0;

    /**
     * Called when passing xml content.  When the content value is transient,
     * the value is not expected to survive beyond the scope of the callback.
     *
     * @param str content value.
     * @param transient whether or not the value is transient.
     */
    virtual void characters(const pstring& str, bool transient) = 0;

    void set_ns_context(const xmlns_context* p);

    const config& get_config() const;

    void set_config(const config& opt);

    void transfer_common(const xml_context_base& parent);

    void set_always_allowed_elements(xml_elem_set_t elems);

protected:
    session_context& get_session_context();
    const tokens& get_tokens() const;
    xml_token_pair_t push_stack(xmlns_id_t ns, xml_token_t name);
    bool pop_stack(xmlns_id_t ns, xml_token_t name);
    xml_token_pair_t& get_current_element();
    const xml_token_pair_t& get_current_element() const;
    xml_token_pair_t& get_parent_element();
    const xml_token_pair_t& get_parent_element() const;
    void warn_unhandled() const;
    void warn_unexpected() const;
    void warn(const char* msg) const;

    /**
     * Check if observed element equals expected element.  If not, it throws an
     * xml_structure_error exception.
     *
     * @param elem element observed.
     * @param ns namespace of expected element.
     * @param name name of expected element.
     * @param error custom error message if needed.
     */
    void xml_element_expected(
        const xml_token_pair_t& elem, xmlns_id_t ns, xml_token_t name,
        const ::std::string* error = nullptr) const;

    void xml_element_expected(
        const xml_token_pair_t& elem, const xml_elem_stack_t& expected_elems) const;

    void xml_element_expected(
        const xml_token_pair_t& elem, const xml_elem_set_t& expected_elems) const;

    void print_namespace(std::ostream& os, xmlns_id_t ns) const;

    void print_current_element_stack(std::ostream& os) const;

    /**
     * Throw a viewer-friendly XML structure error with the information about an
     * unknown element encountered.
     *
     * @param elem unknown element encountered.
     */
    void throw_unknown_element_error(const xml_token_pair_t& elem) const;

    pstring intern(const xml_token_attr_t& attr);
    pstring intern(const pstring& s);

private:
    config m_config;
    const xmlns_context* mp_ns_cxt;
    session_context& m_session_cxt;
    const tokens& m_tokens;
    xml_elem_stack_t m_stack;
    xml_elem_set_t m_always_allowed_elements;
};


}

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
