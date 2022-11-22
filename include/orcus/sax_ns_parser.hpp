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

#include <unordered_set>
#include <vector>
#include <algorithm>

namespace orcus {

struct sax_ns_parser_element
{
    /** Element namespace identifier. */
    xmlns_id_t ns;
    /** Element namespace alias. */
    std::string_view ns_alias;
    /** Element name. */
    std::string_view name;
    /** Position of the opening brace '<'. */
    std::ptrdiff_t begin_pos;
    /** Position immediately after the closing brace '>'. */
    std::ptrdiff_t end_pos;
};

struct sax_ns_parser_attribute
{
    /** Attribute namespace identifier. */
    xmlns_id_t ns;
    /** Attribute namespace alias. */
    std::string_view ns_alias;
    /** Attribute name. */
    std::string_view name;
    /** Attribute value. */
    std::string_view value;
    /** Whether or not the attribute value is transient. */
    bool transient;
};

namespace sax { namespace detail {

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

    elem_scope() {}
    elem_scope(const elem_scope&) = delete;
    elem_scope(elem_scope&& other) = default;
};

using elem_scopes_type = std::vector<elem_scope>;

}} // namespace sax::detail

class sax_ns_handler
{
public:
    /**
     * Called when a doctype declaration &lt;!DOCTYPE ... &gt; is encountered.
     *
     * @param dtd struct containing doctype declaration data.
     */
    void doctype(const orcus::sax::doctype_declaration& dtd)
    {
        (void)dtd;
    }

    /**
     * Called when &lt;?... is encountered, where the '...' may be an
     * arbitraray dentifier.  One common declaration is &lt;?xml which is
     * typically given at the start of an XML stream.
     *
     * @param decl name of the identifier.
     */
    void start_declaration(std::string_view decl)
    {
        (void)decl;
    }

    /**
     * Called when the closing tag (&gt;) of a &lt;?... ?&gt; is encountered.
     *
     * @param decl name of the identifier.
     */
    void end_declaration(std::string_view decl)
    {
        (void)decl;
    }

    /**
     * Called at the start of each element.
     *
     * @param elem information of the element being parsed.
     */
    void start_element(const orcus::sax_ns_parser_element& elem)
    {
        (void)elem;
    }

    /**
     * Called at the end of each element.
     *
     * @param elem information of the element being parsed.
     */
    void end_element(const orcus::sax_ns_parser_element& elem)
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
        (void)val;
        (void)transient;
    }

    /**
     * Called upon parsing of an attribute of a declaration.  The value of an
     * attribute is assumed to be transient thus should be consumed within the
     * scope of this callback.
     *
     * @param name name of an attribute.
     * @param val value of an attribute.
     *
     * @todo Perhaps we should pass the transient flag here as well like all the
     *       other places.
     */
    void attribute(std::string_view name, std::string_view val)
    {
        (void)name;
        (void)val;
    }

    /**
     * Called upon parsing of an attribute of an element.  Note that <em>when
     * the attribute's transient flag is set, the attribute value is stored in
     * a temporary buffer due to a presence of encoded characters, and must be
     * processed within the scope of the callback</em>.
     *
     * @param attr struct containing attribute information.
     */
    void attribute(const orcus::sax_ns_parser_attribute& attr)
    {
        (void)attr;
    }
};

/**
 * SAX based XML parser with extra namespace handling.
 *
 * It uses an instance of xmlns_context passed by the caller to validate and
 * convert namespace values into identifiers.  The namespace identifier of
 * each encountered element is always given even if one is not explicitly
 * given.
 *
 * This parser keeps track of element scopes and detects non-matching element
 * pairs.
 *
 * @tparam HandlerT Handler type with member functions for event callbacks.
 *         Refer to @ref sax_ns_handler.
 */
template<typename HandlerT>
class sax_ns_parser
{
public:
    typedef HandlerT handler_type;

    sax_ns_parser(const char* content, const size_t size, xmlns_context& ns_cxt, handler_type& handler);
    ~sax_ns_parser() = default;

    /**
     * Start parsing the document.
     *
     * @exception orcus::malformed_xml_error when it encounters a
     *                 non-matching closing element.
     */
    void parse();

private:
    /**
     * Re-route callbacks from the internal sax_parser into sax_ns_parser
     * callbacks.
     */
    class handler_wrapper
    {
        sax::detail::elem_scopes_type m_scopes;
        sax::detail::ns_keys_type m_ns_keys;
        sax::detail::entity_names_type m_attrs;

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
            m_scopes.emplace_back();
            sax::detail::elem_scope& scope = m_scopes.back();
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
            sax::detail::elem_scope& scope = m_scopes.back();
            if (scope.ns != m_ns_cxt.get(elem.ns) || scope.name != elem.name)
                throw malformed_xml_error("mis-matching closing element.", -1);

            m_elem.ns = scope.ns;
            m_elem.ns_alias = elem.ns;
            m_elem.name = scope.name;
            m_elem.begin_pos = elem.begin_pos;
            m_elem.end_pos = elem.end_pos;
            m_handler.end_element(m_elem);

            // Pop all namespaces declared in this scope.
            for (const std::string_view& key : scope.ns_keys)
                m_ns_cxt.pop(key);

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

            if (m_attrs.count(sax::detail::entity_name(attr.ns, attr.name)) > 0)
                throw malformed_xml_error(
                    "You can't define two attributes of the same name in the same element.", -1);

            m_attrs.insert(sax::detail::entity_name(attr.ns, attr.name));

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

template<typename HandlerT>
sax_ns_parser<HandlerT>::sax_ns_parser(
    const char* content, const size_t size, xmlns_context& ns_cxt, handler_type& handler) :
    m_wrapper(ns_cxt, handler), m_parser(content, size, m_wrapper)
{
}

template<typename HandlerT>
void sax_ns_parser<HandlerT>::parse()
{
    m_parser.parse();
}

}

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
