/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_ORCUS_THREADED_SAX_TOKEN_PARSER_HPP
#define INCLUDED_ORCUS_THREADED_SAX_TOKEN_PARSER_HPP

#include "tokens.hpp"
#include "xml_namespace.hpp"
#include "sax_token_parser_thread.hpp"
#include "sax_parser_base.hpp"
#include "exception.hpp"
#include "detail/thread.hpp"

#include <thread>

namespace orcus {

class xmlns_context;
class string_pool;

template<typename _Handler>
class threaded_sax_token_parser
{
public:

    typedef _Handler handler_type;

    /**
     * Constructor.
     *
     * @param p pointer to a string stream containing XML content.
     * @param n size of the stream.
     * @param tks XML token map instance.
     * @param ns_cxt namespace context instance.
     * @param hdl handler class instance.
     * @param min_token_size minimum size of the internal token buffer.
     */
    threaded_sax_token_parser(
        const char* p, size_t n, const tokens& tks, xmlns_context& ns_cxt,
        handler_type& hdl, size_t min_token_size);

    /**
     * Constructor.
     *
     * @param p pointer to a string stream containing XML content.
     * @param n size of the stream.
     * @param tks XML token map instance.
     * @param ns_cxt namespace context instance.
     * @param hdl handler class instance.
     * @param min_token_size minimum size of the internal token buffer.
     * @param max_token_size maximum size of the internal token buffer.
     */
    threaded_sax_token_parser(
        const char* p, size_t n, const tokens& tks, xmlns_context& ns_cxt,
        handler_type& hdl, size_t min_token_size, size_t max_token_size);

    /**
     * Call this method to start parsing.
     */
    void parse();

    void swap_string_pool(string_pool& pool);

private:
    void thread_parse();

    void process_tokens(const sax::parse_tokens_t& tokens);

private:
    sax::parser_thread m_parser_thread;
    handler_type& m_handler;
};

template<typename _Handler>
threaded_sax_token_parser<_Handler>::threaded_sax_token_parser(
    const char* p, size_t n, const tokens& tks, xmlns_context& ns_cxt,
    handler_type& hdl, size_t min_token_size) :
    m_parser_thread(p, n, tks, ns_cxt, min_token_size), m_handler(hdl) {}

template<typename _Handler>
threaded_sax_token_parser<_Handler>::threaded_sax_token_parser(
    const char* p, size_t n, const tokens& tks, xmlns_context& ns_cxt, handler_type& hdl,
    size_t min_token_size, size_t max_token_size) :
    m_parser_thread(p, n, tks, ns_cxt, min_token_size, max_token_size), m_handler(hdl) {}

template<typename _Handler>
void threaded_sax_token_parser<_Handler>::parse()
{
    std::thread t(&threaded_sax_token_parser::thread_parse, this);
    detail::thread::scoped_guard guard(std::move(t));

    sax::parse_tokens_t tokens;

    try
    {
        while (m_parser_thread.next_tokens(tokens))
            process_tokens(tokens);

        process_tokens(tokens);
    }
    catch (const std::exception&)
    {
        m_parser_thread.abort();
        throw;
    }
}

template<typename _Handler>
void threaded_sax_token_parser<_Handler>::swap_string_pool(string_pool& pool)
{
    m_parser_thread.swap_string_pool(pool);
}

template<typename _Handler>
void threaded_sax_token_parser<_Handler>::thread_parse()
{
    // Start parsing.
    m_parser_thread.start();
}

template<typename _Handler>
void threaded_sax_token_parser<_Handler>::process_tokens(const sax::parse_tokens_t& tks)
{
    for (const sax::parse_token& t : tks)
    {
        switch (t.type)
        {
            case sax::parse_token_t::start_element:
            {
                const auto* elem = std::get<const xml_token_element_t*>(t.value);
                m_handler.start_element(*elem);
                break;
            }
            case sax::parse_token_t::end_element:
            {
                const auto* elem = std::get<const xml_token_element_t*>(t.value);
                m_handler.end_element(*elem);
                break;
            }
            case sax::parse_token_t::characters:
            {
                auto s = std::get<std::string_view>(t.value);
                m_handler.characters(s, false);
                break;
            }
            case sax::parse_token_t::parse_error:
            {
                auto v = std::get<parse_error_value_t>(t.value);
                throw malformed_xml_error(std::string{v.str}, v.offset);
            }
            default:
                throw general_error("unknown token type encountered.");
        }
    }
}

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
