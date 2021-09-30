/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "orcus/sax_token_parser_thread.hpp"
#include "orcus/global.hpp"
#include "orcus/sax_token_parser.hpp"
#include "orcus/string_pool.hpp"
#include "orcus/detail/parser_token_buffer.hpp"
#include "orcus/tokens.hpp"
#include "orcus/xml_namespace.hpp"

#include <limits>
#include <iostream>

namespace orcus { namespace sax {

parse_token::parse_token() : type(parse_token_t::unknown) {}

parse_token::parse_token(std::string_view _characters) :
    type(parse_token_t::characters),
    value(_characters)
{
}

parse_token::parse_token(parse_token_t _type, const xml_token_element_t* _element) :
    type(_type), value(_element)
{
}

parse_token::parse_token(std::string_view msg, std::ptrdiff_t offset) :
    type(parse_token_t::parse_error),
    value(parse_error_value_t{msg, offset})
{
}

parse_token::parse_token(const parse_token& other) :
    type(other.type), value(other.value)
{
}

bool parse_token::operator== (const parse_token& other) const
{
    return type == other.type && value == other.value;
}

bool parse_token::operator!= (const parse_token& other) const
{
    return !operator==(other);
}

struct parser_thread::impl
{
    detail::thread::parser_token_buffer<parse_tokens_t> m_token_buffer;
    string_pool m_pool;
    std::vector<std::unique_ptr<xml_token_element_t>> m_element_store;

    parse_tokens_t m_parser_tokens; // token buffer for the parser thread.

    const char* mp_char;
    size_t m_size;
    const tokens& m_tokens;
    xmlns_context& m_ns_cxt;

    impl(const char* p, size_t n, const tokens& tks, xmlns_context& ns_cxt, size_t min_token_size, size_t max_token_size) :
        m_token_buffer(min_token_size, max_token_size),
        mp_char(p), m_size(n), m_tokens(tks), m_ns_cxt(ns_cxt)
    {
    }

    void check_and_notify()
    {
        m_token_buffer.check_and_notify(m_parser_tokens);
    }

    void notify_and_finish()
    {
        m_token_buffer.notify_and_finish(m_parser_tokens);
    }

    void abort()
    {
        m_token_buffer.abort();
    }

    void declaration(const orcus::xml_declaration_t& decl)
    {
    }

    void start_element(const orcus::xml_token_element_t& elem)
    {
        m_element_store.emplace_back(std::make_unique<orcus::xml_token_element_t>(elem));
        orcus::xml_token_element_t& this_elem = *m_element_store.back();

        // Go through all attributes and intern transient strings.
        std::for_each(this_elem.attrs.begin(), this_elem.attrs.end(),
            [&](xml_token_attr_t& attr)
            {
                if (attr.transient)
                {
                    attr.value = m_pool.intern(attr.value).first;
                    attr.transient = false;
                }
            }
        );

        m_parser_tokens.emplace_back(parse_token_t::start_element, m_element_store.back().get());
        check_and_notify();
    }

    void end_element(const orcus::xml_token_element_t& elem)
    {
        assert(elem.attrs.empty());

        m_element_store.emplace_back(std::make_unique<orcus::xml_token_element_t>(elem));
        m_parser_tokens.emplace_back(parse_token_t::end_element, m_element_store.back().get());
        check_and_notify();
    }

    void characters(std::string_view val, bool transient)
    {
        if (transient)
            m_parser_tokens.emplace_back(m_pool.intern(val).first);
        else
            m_parser_tokens.emplace_back(val);

        check_and_notify();
    }

    void start()
    {
        try
        {
            try
            {
                orcus::sax_token_parser<impl> parser(mp_char, m_size, m_tokens, m_ns_cxt, *this);
                parser.parse();
            }
            catch (const malformed_xml_error& e)
            {
                std::string_view s = m_pool.intern(e.what()).first;
                m_parser_tokens.emplace_back(s, e.offset());
            }

            // TODO : add more exceptions that need to be tokenized and processed by the client thread.

            notify_and_finish();
        }
        catch (const orcus::detail::parsing_aborted_error&)
        {
            // This is used only to abort the parsing thread prematurely.
        }
    }

    bool next_tokens(parse_tokens_t& tokens)
    {
        return m_token_buffer.next_tokens(tokens);
    }

    void swap_string_pool(string_pool& pool)
    {
        m_pool.swap(pool);
    }
};

parser_thread::parser_thread(
    const char* p, size_t n, const orcus::tokens& tks, xmlns_context& ns_cxt, size_t min_token_size) :
    mp_impl(std::make_unique<parser_thread::impl>(
        p, n, tks, ns_cxt, min_token_size, std::numeric_limits<size_t>::max()/2)) {}

parser_thread::parser_thread(
    const char* p, size_t n, const orcus::tokens& tks, xmlns_context& ns_cxt, size_t min_token_size, size_t max_token_size) :
    mp_impl(std::make_unique<parser_thread::impl>(
        p, n, tks, ns_cxt, min_token_size, max_token_size)) {}

parser_thread::~parser_thread()
{
}

void parser_thread::start()
{
    mp_impl->start();
}

bool parser_thread::next_tokens(parse_tokens_t& tokens)
{
    return mp_impl->next_tokens(tokens);
}

void parser_thread::swap_string_pool(string_pool& pool)
{
    mp_impl->swap_string_pool(pool);
}

void parser_thread::abort()
{
    mp_impl->abort();
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
