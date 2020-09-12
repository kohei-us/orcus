/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef ORCUS_SAX_PARSER_HPP
#define ORCUS_SAX_PARSER_HPP

#include "sax_parser_base.hpp"

namespace orcus {

struct sax_parser_default_config
{
    /**
     * An integer value representing a baseline XML version.  A value of 10
     * corresponds with version 1.0 whereas a value of 11 corresponds with
     * version 1.1.
     */
    static const uint8_t baseline_version = 10;
};

class sax_handler
{
public:
    /**
     * Called when a doctype declaration &lt;!DOCTYPE ... &gt; is encountered.
     *
     * @param param struct containing doctype declaration data.
     */
    void doctype(const orcus::sax::doctype_declaration& param)
    {
        (void)param;
    }

    /**
     * Called when &lt;?... is encountered, where the '...' may be an
     * arbitraray dentifier.  One common declaration is &lt;?xml which is
     * typically given at the start of an XML stream.
     *
     * @param decl name of the identifier.
     */
    void start_declaration(const orcus::pstring& decl)
    {
        (void)decl;
    }

    /**
     * Called when the closing tag (&gt;) of a &lt;?... ?&gt; is encountered.
     *
     * @param decl name of the identifier.
     */
    void end_declaration(const orcus::pstring& decl)
    {
        (void)decl;
    }

    /**
     * Called at the start of each element.
     *
     * @param elem information of the element being parsed.
     */
    void start_element(const orcus::sax::parser_element& elem)
    {
        (void)elem;
    }

    /**
     * Called at the end of each element.
     *
     * @param elem information of the element being parsed.
     */
    void end_element(const orcus::sax::parser_element& elem)
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
    void characters(const orcus::pstring& val, bool transient)
    {
        (void)val; (void)transient;
    }

    /**
     * Called upon parsing of an attribute of an element.  Note that <em>when
     * the attribute's transient flag is set, the attribute value is stored in
     * a temporary buffer due to presence of one or more encoded characters,
     * and must be processed within the scope of the callback</em>.
     *
     * @param attr struct containing attribute information.
     */
    void attribute(const orcus::sax::parser_attribute& attr)
    {
        (void)attr;
    }
};

/**
 * Template-based sax parser that doesn't use function pointer for
 * callbacks for better performance, especially on large XML streams.
 */
template<typename _Handler, typename _Config = sax_parser_default_config>
class sax_parser : public sax::parser_base
{
public:
    typedef _Handler handler_type;
    typedef _Config config_type;

    sax_parser(const char* content, const size_t size, handler_type& handler);
    sax_parser(const char* content, const size_t size, bool transient_stream, handler_type& handler);
    ~sax_parser();

    void parse();

private:

    /**
     * Parse XML header that occurs at the beginning of every XML stream i.e.
     * <?xml version="..." encoding="..." ?>
     */
    void header();
    void body();
    void element();
    void element_open(std::ptrdiff_t begin_pos);
    void element_close(std::ptrdiff_t begin_pos);
    void special_tag();
    void declaration(const char* name_check);
    void cdata();
    void doctype();
    void characters();
    void attribute();

private:
    handler_type& m_handler;
};

template<typename _Handler, typename _Config>
sax_parser<_Handler,_Config>::sax_parser(
    const char* content, const size_t size, handler_type& handler) :
    sax::parser_base(content, size, false),
    m_handler(handler)
{
}

template<typename _Handler, typename _Config>
sax_parser<_Handler,_Config>::sax_parser(
    const char* content, const size_t size, bool transient_stream, handler_type& handler) :
    sax::parser_base(content, size, transient_stream),
    m_handler(handler)
{
}

template<typename _Handler, typename _Config>
sax_parser<_Handler,_Config>::~sax_parser()
{
}

template<typename _Handler, typename _Config>
void sax_parser<_Handler,_Config>::parse()
{
    m_nest_level = 0;
    mp_char = mp_begin;
    header();
    skip_space_and_control();
    body();

    assert(m_buffer_pos == 0);
}

template<typename _Handler, typename _Config>
void sax_parser<_Handler,_Config>::header()
{
    // we don't handle multi byte encodings so we can just skip bom entry if exists.
    skip_bom();
    skip_space_and_control();
    if (!has_char() || cur_char() != '<')
        throw sax::malformed_xml_error("xml file must begin with '<'.", offset());

    if (config_type::baseline_version >= 11)
    {
        // XML version 1.1 requires a header declaration whereas in 1.0 it's
        // optional.
        if (next_char_checked() != '?')
            throw sax::malformed_xml_error("xml file must begin with '<?'.", offset());

        declaration("xml");
    }
}

template<typename _Handler, typename _Config>
void sax_parser<_Handler,_Config>::body()
{
    while (has_char())
    {
        if (cur_char() == '<')
        {
            element();
            if (!m_root_elem_open)
                // Root element closed.  Stop parsing.
                return;
        }
        else if (m_nest_level)
            // Call characters only when in xml hierarchy.
            characters();
        else
            next();
    }
}

template<typename _Handler, typename _Config>
void sax_parser<_Handler,_Config>::element()
{
    assert(cur_char() == '<');
    std::ptrdiff_t pos = offset();
    char c = next_char_checked();
    switch (c)
    {
        case '/':
            element_close(pos);
        break;
        case '!':
            special_tag();
        break;
        case '?':
            declaration(nullptr);
        break;
        default:
            if (!is_alpha(c) && c != '_')
                throw sax::malformed_xml_error("expected an alphabet.", offset());
            element_open(pos);
    }
}

template<typename _Handler, typename _Config>
void sax_parser<_Handler,_Config>::element_open(std::ptrdiff_t begin_pos)
{
    assert(is_alpha(cur_char()) || cur_char() == '_');

    sax::parser_element elem;
    element_name(elem, begin_pos);

    while (true)
    {
        skip_space_and_control();
        char c = cur_char();
        if (c == '/')
        {
            // Self-closing element: <element/>
            if (next_and_char() != '>')
                throw sax::malformed_xml_error("expected '/>' to self-close the element.", offset());
            next();
            elem.end_pos = offset();
            m_handler.start_element(elem);
            reset_buffer_pos();
            m_handler.end_element(elem);
            if (!m_nest_level)
                m_root_elem_open = false;
#if ORCUS_DEBUG_SAX_PARSER
            cout << "element_open: ns='" << elem.ns << "', name='" << elem.name << "' (self-closing)" << endl;
#endif
            return;
        }
        else if (c == '>')
        {
            // End of opening element: <element>
            next();
            elem.end_pos = offset();
            nest_up();
            m_handler.start_element(elem);
            reset_buffer_pos();
#if ORCUS_DEBUG_SAX_PARSER
            cout << "element_open: ns='" << elem.ns << "', name='" << elem.name << "'" << endl;
#endif
            return;
        }
        else
            attribute();
    }
}

template<typename _Handler, typename _Config>
void sax_parser<_Handler,_Config>::element_close(std::ptrdiff_t begin_pos)
{
    assert(cur_char() == '/');
    nest_down();
    next_check();
    sax::parser_element elem;
    element_name(elem, begin_pos);

    if (cur_char() != '>')
        throw sax::malformed_xml_error("expected '>' to close the element.", offset());
    next();
    elem.end_pos = offset();

    m_handler.end_element(elem);
#if ORCUS_DEBUG_SAX_PARSER
    cout << "element_close: ns='" << elem.ns << "', name='" << elem.name << "'" << endl;
#endif
    if (!m_nest_level)
        m_root_elem_open = false;
}

template<typename _Handler, typename _Config>
void sax_parser<_Handler,_Config>::special_tag()
{
    assert(cur_char() == '!');
    // This can be either <![CDATA, <!--, or <!DOCTYPE.
    size_t len = remains();
    if (len < 2)
        throw sax::malformed_xml_error("special tag too short.", offset());

    switch (next_and_char())
    {
        case '-':
        {
            // Possibly comment.
            if (next_and_char() != '-')
                throw sax::malformed_xml_error("comment expected.", offset());

            len -= 2;
            if (len < 3)
                throw sax::malformed_xml_error("malformed comment.", offset());

            next();
            comment();
        }
        break;
        case '[':
        {
            // Possibly a CDATA.
            expects_next("CDATA[", 6);
            if (has_char())
                cdata();
        }
        break;
        case 'D':
        {
            // check if this is a DOCTYPE.
            expects_next("OCTYPE", 6);
            skip_space_and_control();
            if (has_char())
                doctype();
        }
        break;
        default:
            throw sax::malformed_xml_error("failed to parse special tag.", offset());
    }
}

template<typename _Handler, typename _Config>
void sax_parser<_Handler,_Config>::declaration(const char* name_check)
{
    assert(cur_char() == '?');
    next_check();

    // Get the declaration name first.
    pstring decl_name;
    name(decl_name);
#if ORCUS_DEBUG_SAX_PARSER
    cout << "sax_parser::declaration: start name='" << decl_name << "'" << endl;
#endif

    if (name_check && decl_name != name_check)
    {
        std::ostringstream os;
        os << "declaration name of '" << name_check << "' was expected, but '" << decl_name << "' was found instead.";
        throw sax::malformed_xml_error(os.str(), offset());
    }

    m_handler.start_declaration(decl_name);
    skip_space_and_control();

    // Parse the attributes.
    while (cur_char_checked() != '?')
    {
        attribute();
        skip_space_and_control();
    }
    if (next_char_checked() != '>')
        throw sax::malformed_xml_error("declaration must end with '?>'.", offset());

    m_handler.end_declaration(decl_name);
    reset_buffer_pos();
    next();
#if ORCUS_DEBUG_SAX_PARSER
    cout << "sax_parser::declaration: end name='" << decl_name << "'" << endl;
#endif
}

template<typename _Handler, typename _Config>
void sax_parser<_Handler,_Config>::cdata()
{
    size_t len = remains();
    assert(len > 3);

    // Parse until we reach ']]>'.
    const char* p0 = mp_char;
    size_t i = 0, match = 0;
    for (char c = cur_char(); i < len; ++i, c = next_and_char())
    {
        if (c == ']')
        {
            // Be aware that we may encounter a series of more than two ']'
            // characters, in which case we'll only count the last two.

            if (match == 0)
                // First ']'
                ++match;
            else if (match == 1)
                // Second ']'
                ++match;
        }
        else if (c == '>' && match == 2)
        {
            // Found ']]>'.
            size_t cdata_len = i - 2;
            m_handler.characters(pstring(p0, cdata_len), transient_stream());
            next();
            return;
        }
        else
            match = 0;
    }
    throw sax::malformed_xml_error("malformed CDATA section.", offset());
}

template<typename _Handler, typename _Config>
void sax_parser<_Handler,_Config>::doctype()
{
    // Parse the root element first.
    sax::doctype_declaration param;
    name(param.root_element);
    skip_space_and_control();

    // Either PUBLIC or SYSTEM.
    size_t len = remains();
    if (len < 6)
        throw sax::malformed_xml_error("DOCTYPE section too short.", offset());

    param.keyword = sax::doctype_declaration::keyword_type::dtd_private;
    char c = cur_char();
    if (c == 'P')
    {
        if (next_and_char() != 'U' || next_and_char() != 'B' || next_and_char() != 'L' || next_and_char() != 'I' || next_and_char() != 'C')
            throw sax::malformed_xml_error("malformed DOCTYPE section.", offset());

        param.keyword = sax::doctype_declaration::keyword_type::dtd_public;
    }
    else if (c == 'S')
    {
        if (next_and_char() != 'Y' || next_and_char() != 'S' || next_and_char() != 'T' || next_and_char() != 'E' || next_and_char() != 'M')
            throw sax::malformed_xml_error("malformed DOCTYPE section.", offset());
    }

    next_check();
    skip_space_and_control();
    has_char_throw("DOCTYPE section too short.");

    // Parse FPI.
    value(param.fpi, false);

    has_char_throw("DOCTYPE section too short.");
    skip_space_and_control();
    has_char_throw("DOCTYPE section too short.");

    if (cur_char() == '>')
    {
        // Optional URI not given. Exit.
#if ORCUS_DEBUG_SAX_PARSER
        cout << "sax_parser::doctype: root='" << param.root_element << "', fpi='" << param.fpi << "'" << endl;
#endif
        m_handler.doctype(param);
        next();
        return;
    }

    // Parse optional URI.
    value(param.uri, false);

    has_char_throw("DOCTYPE section too short.");
    skip_space_and_control();
    has_char_throw("DOCTYPE section too short.");

    if (cur_char() != '>')
        throw sax::malformed_xml_error("malformed DOCTYPE section - closing '>' expected but not found.", offset());

#if ORCUS_DEBUG_SAX_PARSER
    cout << "sax_parser::doctype: root='" << param.root_element << "', fpi='" << param.fpi << "' uri='" << param.uri << "'" << endl;
#endif
    m_handler.doctype(param);
    next();
}

template<typename _Handler, typename _Config>
void sax_parser<_Handler,_Config>::characters()
{
    const char* p0 = mp_char;
    for (; has_char(); next())
    {
        if (cur_char() == '<')
            break;

        if (cur_char() == '&')
        {
            // Text span with one or more encoded characters. Parse using cell buffer.
            cell_buffer& buf = get_cell_buffer();
            buf.reset();
            buf.append(p0, mp_char-p0);
            characters_with_encoded_char(buf);
            if (buf.empty())
                m_handler.characters(pstring(), transient_stream());
            else
                m_handler.characters(pstring(buf.get(), buf.size()), true);
            return;
        }
    }

    if (mp_char > p0)
    {
        pstring val(p0, mp_char-p0);
        m_handler.characters(val, transient_stream());
    }
}

template<typename _Handler, typename _Config>
void sax_parser<_Handler,_Config>::attribute()
{
    sax::parser_attribute attr;
    pstring attr_ns_name, attr_name, attr_value;
    attribute_name(attr.ns, attr.name);

#if ORCUS_DEBUG_SAX_PARSER
    cout << "sax_parser::attribute: ns='" << attr.ns << "', name='" << attr.name << "'" << endl;
#endif

    skip_space_and_control();

    char c = cur_char();
    if (c != '=')
    {
        std::ostringstream os;
        os << "Attribute must begin with 'name=..'. (ns='" << attr.ns << "', name='" << attr.name << "')";
        throw sax::malformed_xml_error(os.str(), offset());
    }

    next_check(); // skip the '='.
    skip_space_and_control();

    attr.transient = value(attr.value, true);
    if (attr.transient)
        // Value is stored in a temporary buffer. Push a new buffer.
        inc_buffer_pos();

#if ORCUS_DEBUG_SAX_PARSER
    cout << "sax_parser::attribute: value='" << attr.value << "'" << endl;
#endif

    m_handler.attribute(attr);
}

}

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
