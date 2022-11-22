/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_ORCUS_SAX_TOKEN_PARSER_HPP
#define INCLUDED_ORCUS_SAX_TOKEN_PARSER_HPP

#include "sax_ns_parser.hpp"
#include "types.hpp"

#include <vector>
#include <algorithm>
#include <functional>

namespace orcus {

class tokens;

namespace sax {

#if ORCUS_DEBUG_SAX_PARSER
template<typename _Attr, typename _Tokens>
class attr_printer
{
public:
    attr_printer(const _Tokens& tokens, const ::std::string& indent) :
        m_tokens(tokens), m_indent(indent) {}

    void operator() (const _Attr& attr) const
    {
        using namespace std;
        cout << m_indent << "  attribute: "
            << attr.ns << ":"
            << m_tokens.get_token_name(attr.name) << "=\""
            << attr.value.str() << "\"" << endl;
    }
private:
    const _Tokens& m_tokens;
    ::std::string m_indent;
};
#endif

}

class ORCUS_PSR_DLLPUBLIC sax_token_handler_wrapper_base
{
protected:
    xml_declaration_t m_declaration;
    xml_token_element_t m_elem;
    const tokens& m_tokens;

    xml_token_t tokenize(std::string_view name) const;
    void set_element(const sax_ns_parser_element& elem);

public:
    sax_token_handler_wrapper_base(const tokens& _tokens);

    void attribute(std::string_view name, std::string_view val);
    void attribute(const sax_ns_parser_attribute& attr);
};

class sax_token_handler
{
public:

    /**
     * Called immediately after the entire XML declaration has been parsed.
     *
     * @param decl struct containing the attributes of the XML declaration.
     */
    void declaration(const orcus::xml_declaration_t& decl)
    {
        (void)decl;
    }

    /**
     * Called at the start of each element.
     *
     * @param elem struct containing the element's information as well as all
     *             the attributes that belong to the element.
     */
    void start_element(const orcus::xml_token_element_t& elem)
    {
        (void)elem;
    }

    /**
     * Called at the end of each element.
     *
     * @param elem struct containing the element's information as well as all
     *             the attributes that belong to the element.
     */
    void end_element(const orcus::xml_token_element_t& elem)
    {
        (void)elem;
    }

    /**
     * Called when a segment of a text content is parsed.  Each text content
     * is a direct child of an element, which may have multiple child contents
     * when the element also has a child element that are direct sibling to
     * the text contents or the text contents are splitted by a comment.
     *
     * @param val value of the text content.
     * @param transient when true, the text content has been converted and is
     *                  stored in a temporary buffer due to presence of one or
     *                  more encoded characters, in which case <em>the passed
     *                  text value needs to be either immediately converted to
     *                  a non-text value or be interned within the scope of
     *                  the callback</em>.
     */
    void characters(std::string_view val, bool transient)
    {
        (void)val; (void)transient;
    }
};

/**
 * SAX parser that tokenizes element and attribute names while parsing. All
 * pre-defined elements and attribute names are translated into integral
 * identifiers via use of @ref tokens.  The user of this class needs to
 * provide a pre-defined set of element and attribute names at construction
 * time.
 *
 * This parser internally uses @ref sax_ns_parser.
 *
 * @tparam HandlerT Handler type with member functions for event callbacks.
 *         Refer to @ref sax_token_handler.
 */
template<typename HandlerT>
class sax_token_parser
{
public:
    typedef HandlerT handler_type;

    sax_token_parser(
        const char* content, const size_t size, const tokens& _tokens,
        xmlns_context& ns_cxt, handler_type& handler);

    ~sax_token_parser() = default;

    void parse();

private:

    /**
     * Re-route callbacks from the internal sax_ns_parser into the
     * sax_token_parser callbacks.
     */
    class handler_wrapper : public sax_token_handler_wrapper_base
    {
        handler_type& m_handler;

    public:
        handler_wrapper(const tokens& _tokens, handler_type& handler) :
            sax_token_handler_wrapper_base(_tokens), m_handler(handler) {}

        void doctype(const sax::doctype_declaration&) {}

        void start_declaration(std::string_view) {}

        void end_declaration(std::string_view)
        {
            m_handler.declaration(m_declaration);
            m_elem.attrs.clear();
        }

        void start_element(const sax_ns_parser_element& elem)
        {
            set_element(elem);
            m_handler.start_element(m_elem);
            m_elem.attrs.clear();
        }

        void end_element(const sax_ns_parser_element& elem)
        {
            set_element(elem);
            m_handler.end_element(m_elem);
        }

        void characters(std::string_view val, bool transient)
        {
            m_handler.characters(val, transient);
        }
    };

private:
    handler_wrapper m_wrapper;
    sax_ns_parser<handler_wrapper> m_parser;
};

template<typename HandlerT>
sax_token_parser<HandlerT>::sax_token_parser(
    const char* content, const size_t size, const tokens& _tokens, xmlns_context& ns_cxt, handler_type& handler) :
    m_wrapper(_tokens, handler),
    m_parser(content, size, ns_cxt, m_wrapper)
{
}

template<typename HandlerT>
void sax_token_parser<HandlerT>::parse()
{
    m_parser.parse();
}

} // namespace orcus

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
