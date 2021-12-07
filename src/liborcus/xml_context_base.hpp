/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_ORCUS_XML_CONTEXT_BASE_HPP
#define INCLUDED_ORCUS_XML_CONTEXT_BASE_HPP

#include "xml_stream_handler.hpp"

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

    /**
     * This gets called at the end of the initial XML declaration block i.e.
     * &lt;?xml ... ?&gt;.  Obviously this gets called only on the root context.
     *
     * @param decl XML declaration attributes
     */
    virtual void declaration(const xml_declaration_t& decl);

    /**
     * This method gets called by the stream handler to fetch a child context
     * object if applicable. If the current context can handle the specified
     * element, it should return nullptr.  If the current context should spawn a
     * new context to handle the specified element and its sub structure, then
     * it should return a pointer to a child context.
     *
     * @note The caller is not responsible for managing the life cycle of the
     * returned context object; the current context object must manage the life
     * cycle of the context object it returns.
     *
     * @param ns namespace value for the element.
     * @param name name of the element.
     *
     * @return pointer to the context object that should handle the specified
     *         element, or nullptr if the current context can handle the
     *         element.
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

    /**
     * Called on the opening of each element.
     *
     * @param ns namespace token
     * @param name element name
     * @param attrs attributes
     */
    virtual void start_element(xmlns_id_t ns, xml_token_t name, const std::vector<xml_token_attr_t>& attrs) = 0;

    /**
     * Called on the closing of each element.
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
    virtual void characters(std::string_view str, bool transient) = 0;

    virtual bool evaluate_child_element(xmlns_id_t ns, xml_token_t name) const;

    void set_ns_context(const xmlns_context* p);

    const config& get_config() const;

    void set_config(const config& opt);

    void transfer_common(const xml_context_base& parent);

    void set_always_allowed_elements(xml_elem_set_t elems);

    xml_context_base* get_invalid_element_context();

protected:
    session_context& get_session_context();
    const tokens& get_tokens() const;
    xml_token_pair_t push_stack(xmlns_id_t ns, xml_token_t name);
    bool pop_stack(xmlns_id_t ns, xml_token_t name);
    xml_token_pair_t get_current_stack(xmlns_id_t ns, xml_token_t name);
    xml_token_pair_t get_current_element() const;
    const xml_token_pair_t& get_parent_element() const;
    void warn_unhandled() const;
    void warn_unexpected() const;
    void warn(std::string_view msg) const;

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

    bool xml_element_always_allowed(const xml_token_pair_t& elem) const;

    void print_namespace(std::ostream& os, xmlns_id_t ns) const;

    void print_element(std::ostream& os, const xml_token_pair_t& elem) const;

    void print_current_element_stack(std::ostream& os) const;

    /**
     * Throw a viewer-friendly XML structure error with the information about an
     * unknown element encountered.
     *
     * @param elem unknown element encountered.
     */
    void throw_unknown_element_error(const xml_token_pair_t& elem) const;

    std::string_view intern(const xml_token_attr_t& attr);
    std::string_view intern(std::string_view s);

private:
    config m_config;
    const xmlns_context* mp_ns_cxt;
    session_context& m_session_cxt;
    const tokens& m_tokens;
    xml_elem_stack_t m_stack;
    xml_elem_set_t m_always_allowed_elements;

    std::unique_ptr<xml_context_base> m_empty_cxt;
};


}

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
