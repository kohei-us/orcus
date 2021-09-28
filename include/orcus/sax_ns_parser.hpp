/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_ORCUS_SAX_NS_PARSER_HPP
#define INCLUDED_ORCUS_SAX_NS_PARSER_HPP

#include "sax_parser.hpp"
#include "xml_namespace.hpp"
#include "global.hpp"

#include <unordered_set>
#include <vector>
#include <memory>
#include <algorithm>

namespace orcus {

struct sax_ns_parser_element
{
    xmlns_id_t ns;         // element namespace
    std::string_view ns_alias;      // element namespace alias
    std::string_view name;          // element name
    std::ptrdiff_t begin_pos; // position of the opening brace '<'.
    std::ptrdiff_t end_pos;   // position of the char after the closing brace '>'.
};

struct sax_ns_parser_attribute
{
    xmlns_id_t ns;    // attribute namespace
    std::string_view ns_alias; // attribute namespace alias
    std::string_view name;     // attribute name
    std::string_view value;    // attribute value
    bool transient;   // whether or not the attribute value is transient.
};

namespace __sax {

struct entity_name
{
    std::string_view ns;
    std::string_view name;

    entity_name(std::string_view _ns, std::string_view _name) :
        ns(_ns), name(_name) {}

    bool operator== (const entity_name& other) const
    {
        return other.ns == ns && other.name == name;
    }

    struct hash
    {
        size_t operator() (const entity_name& v) const
        {
            std::hash<std::string_view> hasher;
            return hasher(v.ns) + hasher(v.name);
        }
    };
};

typedef std::unordered_set<std::string_view> ns_keys_type;
typedef std::unordered_set<entity_name, entity_name::hash> entity_names_type;

struct elem_scope
{
    xmlns_id_t ns;
    std::string_view name;
    ns_keys_type ns_keys;
};

typedef std::vector<std::unique_ptr<elem_scope>> elem_scopes_type;

class pop_ns_by_key
{
    xmlns_context& m_cxt;
public:
    pop_ns_by_key(xmlns_context& cxt) : m_cxt(cxt) {}
    void operator() (std::string_view key)
    {
        m_cxt.pop(key);
    }
};

}

class sax_ns_handler
{
public:
    void doctype(const orcus::sax::doctype_declaration& /*dtd*/) {}

    void start_declaration(std::string_view /*decl*/) {}

    void end_declaration(std::string_view /*decl*/) {}

    void start_element(const orcus::sax_ns_parser_element& /*elem*/) {}

    void end_element(const orcus::sax_ns_parser_element& /*elem*/) {}

    void characters(std::string_view /*val*/, bool /*transient*/) {}

    void attribute(std::string_view /*name*/, std::string_view /*val*/) {}

    void attribute(const orcus::sax_ns_parser_attribute& /*attr*/) {}
};

/**
 * SAX based XML parser with proper namespace handling.
 */
template<typename _Handler>
class sax_ns_parser
{
public:
    typedef _Handler handler_type;

    sax_ns_parser(const char* content, const size_t size, xmlns_context& ns_cxt, handler_type& handler);
    sax_ns_parser(const char* content, const size_t size, bool transient_stream,
                  xmlns_context& ns_cxt, handler_type& handler);
    ~sax_ns_parser();

    void parse();

private:
    /**
     * Re-route callbacks from the internal sax_parser into sax_ns_parser
     * callbacks.
     */
    class handler_wrapper
    {
        __sax::elem_scopes_type m_scopes;
        __sax::ns_keys_type m_ns_keys;
        __sax::entity_names_type m_attrs;

        sax_ns_parser_element m_elem;
        sax_ns_parser_attribute m_attr;

        xmlns_context& m_ns_cxt;
        handler_type& m_handler;

        bool m_declaration;

    public:
        handler_wrapper(xmlns_context& ns_cxt, handler_type& handler) : m_ns_cxt(ns_cxt), m_handler(handler), m_declaration(false) {}

        void doctype(const sax::doctype_declaration& dtd)
        {
            m_handler.doctype(dtd);
        }

        void start_declaration(std::string_view name)
        {
            m_declaration = true;
            m_handler.start_declaration(name);
        }

        void end_declaration(std::string_view name)
        {
            m_declaration = false;
            m_handler.end_declaration(name);
        }

        void start_element(const sax::parser_element& elem)
        {
            m_scopes.push_back(std::make_unique<__sax::elem_scope>());
            __sax::elem_scope& scope = *m_scopes.back();
            scope.ns = m_ns_cxt.get(elem.ns);
            scope.name = elem.name;
            scope.ns_keys.swap(m_ns_keys);

            m_elem.ns = scope.ns;
            m_elem.ns_alias = elem.ns;
            m_elem.name = scope.name;
            m_elem.begin_pos = elem.begin_pos;
            m_elem.end_pos = elem.end_pos;
            m_handler.start_element(m_elem);

            m_attrs.clear();
        }

        void end_element(const sax::parser_element& elem)
        {
            __sax::elem_scope& scope = *m_scopes.back();
            if (scope.ns != m_ns_cxt.get(elem.ns) || scope.name != elem.name)
                throw sax::malformed_xml_error("mis-matching closing element.", -1);

            m_elem.ns = scope.ns;
            m_elem.ns_alias = elem.ns;
            m_elem.name = scope.name;
            m_elem.begin_pos = elem.begin_pos;
            m_elem.end_pos = elem.end_pos;
            m_handler.end_element(m_elem);

            // Pop all namespaces declared in this scope.
            std::for_each(scope.ns_keys.begin(), scope.ns_keys.end(), __sax::pop_ns_by_key(m_ns_cxt));

            m_scopes.pop_back();
        }

        void characters(std::string_view val, bool transient)
        {
            m_handler.characters(val, transient);
        }

        void attribute(const sax::parser_attribute& attr)
        {
            if (m_declaration)
            {
                // XML declaration attribute.  Pass it through to the handler without namespace.
                m_handler.attribute(attr.name, attr.value);
                return;
            }

            if (m_attrs.count(__sax::entity_name(attr.ns, attr.name)) > 0)
                throw sax::malformed_xml_error(
                    "You can't define two attributes of the same name in the same element.", -1);

            m_attrs.insert(__sax::entity_name(attr.ns, attr.name));

            if (attr.ns.empty() && attr.name == "xmlns")
            {
                // Default namespace
                m_ns_cxt.push(std::string_view{}, attr.value);
                m_ns_keys.insert(std::string_view{});
                return;
            }

            if (attr.ns == "xmlns")
            {
                // Namespace alias
                if (!attr.name.empty())
                {
                    m_ns_cxt.push(attr.name, attr.value);
                    m_ns_keys.insert(attr.name);
                }
                return;
            }

            m_attr.ns = attr.ns.empty() ? XMLNS_UNKNOWN_ID : m_ns_cxt.get(attr.ns);
            m_attr.ns_alias = attr.ns;
            m_attr.name = attr.name;
            m_attr.value = attr.value;
            m_attr.transient = attr.transient;
            m_handler.attribute(m_attr);
        }
    };

private:
    handler_wrapper m_wrapper;
    sax_parser<handler_wrapper> m_parser;
};

template<typename _Handler>
sax_ns_parser<_Handler>::sax_ns_parser(
    const char* content, const size_t size, xmlns_context& ns_cxt, handler_type& handler) :
    m_wrapper(ns_cxt, handler), m_parser(content, size, m_wrapper)
{
}

template<typename _Handler>
sax_ns_parser<_Handler>::sax_ns_parser(
    const char* content, const size_t size, bool transient_stream, xmlns_context& ns_cxt, handler_type& handler) :
    m_wrapper(ns_cxt, handler), m_parser(content, size, transient_stream, m_wrapper)
{
}

template<typename _Handler>
sax_ns_parser<_Handler>::~sax_ns_parser()
{
}

template<typename _Handler>
void sax_ns_parser<_Handler>::parse()
{
    m_parser.parse();
}

}

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
