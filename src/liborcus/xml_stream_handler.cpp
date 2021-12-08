/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "xml_stream_handler.hpp"
#include "xml_context_base.hpp"
#include "xml_empty_context.hpp"

#include "orcus/exception.hpp"

#include <iostream>

namespace orcus {

xml_stream_handler::xml_stream_handler(
    session_context& session_cxt, const tokens& t, std::unique_ptr<xml_context_base> root_context) :
    m_session_cxt(session_cxt),
    m_tokens(t),
    m_config(format_t::unknown),
    m_elem_printer(m_tokens),
    mp_root_context(std::move(root_context)),
    mp_invalid_context(std::make_unique<xml_empty_context>(session_cxt, t))
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
            m_context_stack.push_back(p);
    }
    else
    {
        // new child element is not valid for the current element. Ignore the
        // whole sub structure.
        m_context_stack.push_back(&get_invalid_context());

        if (m_config.debug)
        {
            // TODO: print the top element of the sub structure being ignored.
            std::cerr << "warning: ignoring the whole sub-structure below ";
            m_elem_printer.print_element(std::cerr, elem.ns, elem.name);
            std::cerr << std::endl;
        }
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
    for (auto* context : m_context_stack)
        context->set_ns_context(p);

    mp_invalid_context->set_ns_context(p);
    m_elem_printer.set_ns_context(p);
}

void xml_stream_handler::set_config(const config& opt)
{
    m_config = opt;
    for (auto* context : m_context_stack)
        context->set_config(m_config);

    mp_invalid_context->set_config(m_config);
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

xml_context_base& xml_stream_handler::get_invalid_context()
{
    return *mp_invalid_context;
}

}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
