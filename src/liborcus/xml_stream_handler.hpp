/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef ORCUS_XML_STREAM_HANDLER_HPP
#define ORCUS_XML_STREAM_HANDLER_HPP

#include <orcus/sax_token_parser.hpp>
#include <orcus/config.hpp>

#include "xml_util.hpp"

#include <cstdlib>
#include <string>
#include <vector>
#include <memory>

namespace orcus {

class xml_context_base;
class xmlns_context;
struct session_context;

class xml_stream_handler
{
    session_context& m_session_cxt;
    const tokens& m_tokens;

    config m_config;
    element_printer m_elem_printer;
    std::unique_ptr<xml_context_base> mp_root_context;
    std::unique_ptr<xml_context_base> mp_invalid_context;
    typedef std::vector<xml_context_base*> context_stack_type;
    context_stack_type m_context_stack;

public:
    xml_stream_handler() = delete;
    xml_stream_handler(const xml_stream_handler&) = delete;

    xml_stream_handler(session_context& session_cxt, const tokens& t, std::unique_ptr<xml_context_base> root_context);
    virtual ~xml_stream_handler();

    virtual void start_document();
    virtual void end_document();

    virtual void declaration(const xml_declaration_t& decl);
    virtual void start_element(const xml_token_element_t& elem);
    virtual void end_element(const xml_token_element_t& elem);
    virtual void characters(std::string_view str, bool transient);

    void set_ns_context(const xmlns_context* p);
    void set_config(const config& opt);

protected:
    xml_context_base& get_current_context();
    xml_context_base& get_root_context();
    xml_context_base& get_invalid_context();
};

}

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
