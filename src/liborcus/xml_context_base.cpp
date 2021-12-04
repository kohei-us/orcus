/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "xml_context_base.hpp"
#include "session_context.hpp"
#include "xml_empty_context.hpp"

#include "orcus/exception.hpp"
#include "orcus/tokens.hpp"

#include <iostream>
#include <sstream>

using namespace std;

namespace orcus {

namespace {

void print_stack(const tokens& tokens, const xml_elem_stack_t& elem_stack, const xmlns_context* ns_cxt)
{
    cerr << "[ ";
    xml_elem_stack_t::const_iterator itr, itr_beg = elem_stack.begin(), itr_end = elem_stack.end();
    for (itr = itr_beg; itr != itr_end; ++itr)
    {
        if (itr != itr_beg)
            cerr << " -> ";

        xmlns_id_t ns = itr->first;
        if (ns_cxt)
        {
            pstring alias = ns_cxt->get_alias(ns);
            if (!alias.empty())
                cerr << alias << ":";
        }
        else
            cerr << ns << ":";

        cerr << tokens.get_token_name(itr->second);
    }
    cerr << " ]";
}

}

xml_context_base::xml_context_base(session_context& session_cxt, const tokens& tokens) :
    m_config(format_t::unknown),
    mp_ns_cxt(nullptr), m_session_cxt(session_cxt), m_tokens(tokens)
{
}

xml_context_base::~xml_context_base()
{
}

void xml_context_base::declaration(const xml_declaration_t& /*decl*/)
{
}

bool xml_context_base::evaluate_child_element(xmlns_id_t /*ns*/, xml_token_t /*name*/) const
{
    return true;
}

void xml_context_base::set_ns_context(const xmlns_context* p)
{
    mp_ns_cxt = p;
}

void xml_context_base::set_config(const config& opt)
{
    m_config = opt;
}

void xml_context_base::transfer_common(const xml_context_base& parent)
{
    m_config = parent.m_config;
    mp_ns_cxt = parent.mp_ns_cxt;
}

void xml_context_base::set_always_allowed_elements(xml_elem_set_t elems)
{
    m_always_allowed_elements = std::move(elems);
}

xml_context_base* xml_context_base::get_empty_context()
{
    if (!m_empty_cxt)
    {
        m_empty_cxt = std::make_unique<xml_empty_context>(m_session_cxt, m_tokens);
        m_empty_cxt->transfer_common(*this);
    }

    return m_empty_cxt.get();
}

session_context& xml_context_base::get_session_context()
{
    return m_session_cxt;
}

const tokens& xml_context_base::get_tokens() const
{
    return m_tokens;
}

xml_token_pair_t xml_context_base::push_stack(xmlns_id_t ns, xml_token_t name)
{
    xml_token_pair_t parent = get_current_element();
    m_stack.emplace_back(ns, name);
    return parent;
}

bool xml_context_base::pop_stack(xmlns_id_t ns, xml_token_t name)
{
    const xml_token_pair_t& r = m_stack.back();

    if (ns != r.first || name != r.second)
        throw general_error("mismatched element name");

    m_stack.pop_back();
    return m_stack.empty();
}

xml_token_pair_t xml_context_base::get_current_element() const
{
    return m_stack.empty() ? xml_token_pair_t(XMLNS_UNKNOWN_ID, XML_UNKNOWN_TOKEN) : m_stack.back();
}

const xml_token_pair_t& xml_context_base::get_parent_element() const
{
    if (m_stack.size() < 2)
        throw general_error("element stack has no parent element");

    return m_stack[m_stack.size() - 2];
}

void xml_context_base::warn_unhandled() const
{
    if (!m_config.debug)
        return;

    cerr << "warning: unhandled element ";
    print_stack(m_tokens, m_stack, mp_ns_cxt);
    cerr << endl;
}

void xml_context_base::warn_unexpected() const
{
    if (!m_config.debug)
        return;

    cerr << "warning: unexpected element ";
    print_stack(m_tokens, m_stack, mp_ns_cxt);
    cerr << endl;
}

void xml_context_base::warn(const char* msg) const
{
    if (!m_config.debug)
        return;

    cerr << "warning: " << msg << endl;
}

void xml_context_base::xml_element_expected(
    const xml_token_pair_t& elem, xmlns_id_t ns, xml_token_t name,
    const string* error) const
{
    if (!m_config.structure_check)
        return;

    if (elem.first == ns && elem.second == name)
        // This is an expected element.  Good.
        return;

    if (m_always_allowed_elements.count(elem))
        return;

    if (error)
    {
        throw xml_structure_error(*error);
    }

    // Create a generic error message.
    std::ostringstream os;
    os << "element <";
    print_namespace(os, ns);
    os << ":" << m_tokens.get_token_name(name) << "> expected, but <";
    print_namespace(os, elem.first);
    os << ":" << m_tokens.get_token_name(elem.second) << "> encountered." << std::endl;
    os << std::endl;

    print_current_element_stack(os);
    throw xml_structure_error(os.str());
}

void xml_context_base::xml_element_expected(
    const xml_token_pair_t& elem, const xml_elem_stack_t& expected_elems) const
{
    if (!m_config.structure_check)
        return;

    for (const xml_token_pair_t& e : expected_elems)
    {
        if (elem == e)
            return;
    }

    if (m_always_allowed_elements.count(elem))
        return;

    throw_unknown_element_error(elem);
}

void xml_context_base::xml_element_expected(
    const xml_token_pair_t& elem, const xml_elem_set_t& expected_elems) const
{
    if (!m_config.structure_check)
        return;

    if (expected_elems.count(elem))
        return;

    if (m_always_allowed_elements.count(elem))
        return;

    throw_unknown_element_error(elem);
}

bool xml_context_base::xml_element_valid(
    const xml_token_pair_t& elem, xmlns_id_t ns, xml_token_t name) const
{
    if (!m_config.structure_check)
        return true;

    if (elem.first == ns && elem.second == name)
        // This is an expected element.  Good.
        return true;

    if (m_always_allowed_elements.count(elem))
        return true;

    return false;
}

bool xml_context_base::xml_element_valid(
    const xml_token_pair_t& elem, const xml_elem_set_t& expected_elems) const
{
    if (!m_config.structure_check)
        return true;

    if (expected_elems.count(elem))
        return true;

    if (m_always_allowed_elements.count(elem))
        return true;

    return false;
}

void xml_context_base::print_namespace(std::ostream& os, xmlns_id_t ns) const
{
    if (mp_ns_cxt)
    {
        std::string_view alias = mp_ns_cxt->get_alias(ns);
        if (!alias.empty())
            os << alias;
    }
    else
        os << ns;
}

void xml_context_base::print_current_element_stack(std::ostream& os) const
{
    os << "current element stack:" << std::endl << std::endl;

    for (const auto& [ns, elem] : m_stack)
    {
        os << "  - <";
        print_namespace(os, ns);
        os << ':' << m_tokens.get_token_name(elem) << ">" << std::endl;
    }
}

void xml_context_base::throw_unknown_element_error(const xml_token_pair_t& elem) const
{
    // Create a generic error message.
    std::ostringstream os;
    os << "unexpected element encountered: ";
    print_namespace(os, elem.first);
    os << ":" << m_tokens.get_token_name(elem.second) << std::endl;
    os << std::endl;

    print_current_element_stack(os);
    throw xml_structure_error(os.str());
}

const config& xml_context_base::get_config() const
{
    return m_config;
}

std::string_view xml_context_base::intern(const xml_token_attr_t& attr)
{
    return m_session_cxt.intern(attr);
}

std::string_view xml_context_base::intern(std::string_view s)
{
    return m_session_cxt.intern(s);
}

}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
