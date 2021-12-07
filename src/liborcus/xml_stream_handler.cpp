/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "xml_stream_handler.hpp"
#include "xml_context_base.hpp"

#include "orcus/exception.hpp"

namespace orcus {

xml_stream_handler::xml_stream_handler(
    session_context& session_cxt, const tokens& t, std::unique_ptr<xml_context_base> root_context) :
    m_session_cxt(session_cxt),
    m_tokens(t),
    m_config(format_t::unknown),
    mp_ns_cxt(nullptr),
    mp_root_context(std::move(root_context))
{
    assert(mp_root_context);
    m_context_stack.push_back(mp_root_context.get());
}

xml_stream_handler::~xml_stream_handler()
{
}

void xml_stream_handler::start_document()
{
}

void xml_stream_handler::end_document()
{
}

void xml_stream_handler::declaration(const xml_declaration_t& decl)
{
    get_current_context().declaration(decl);
}

void xml_stream_handler::start_element(const xml_token_element_t& elem)
{
    xml_context_base& cur = get_current_context();
    if (cur.evaluate_child_element(elem.ns, elem.name))
    {
        // new child element is valid against parent element.
        xml_context_base* p = cur.create_child_context(elem.ns, elem.name);
        if (p)
        {
            m_context_stack.push_back(p);
            m_context_stack.back()->set_ns_context(mp_ns_cxt);
        }
    }
    else
    {
        // new child element is not valid for the current element. Ignore the
        // whole sub structure.
        m_context_stack.push_back(cur.get_invalid_element_context());
        m_context_stack.back()->set_ns_context(mp_ns_cxt);
    }

    get_current_context().start_element(elem.ns, elem.name, elem.attrs);
}

void xml_stream_handler::end_element(const xml_token_element_t& elem)
{
    bool ended = get_current_context().end_element(elem.ns, elem.name);

    if (ended)
    {
        size_t n = m_context_stack.size();

        if (n > 1)
        {
            // Call end_child_context of the parent context to provide a way for
            // the two adjacent contexts to communicate with each other.
            context_stack_type::reverse_iterator itr_cur = m_context_stack.rbegin();
            context_stack_type::reverse_iterator itr_par = itr_cur + 1;
            (*itr_par)->end_child_context(elem.ns, elem.name, *itr_cur);
        }

        m_context_stack.pop_back();
    }
}

void xml_stream_handler::characters(std::string_view str, bool transient)
{
    get_current_context().characters(str, transient);
}

void xml_stream_handler::set_ns_context(const xmlns_context* p)
{
    mp_ns_cxt = p;
    if (!m_context_stack.empty())
        m_context_stack.back()->set_ns_context(p);
}

void xml_stream_handler::set_config(const config& opt)
{
    m_config = opt;
    if (!m_context_stack.empty())
        m_context_stack.back()->set_config(m_config);
}

xml_context_base& xml_stream_handler::get_current_context()
{
    if (m_context_stack.empty())
        return *mp_root_context;

    return *m_context_stack.back();
}

xml_context_base& xml_stream_handler::get_root_context()
{
    return *mp_root_context;
}

}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
