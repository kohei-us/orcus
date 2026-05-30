
//!code-start: headers
#include <orcus/sax_ns_parser.hpp>
#include <orcus/xml_namespace.hpp>

#include <iostream>
//!code-end: headers

using namespace orcus;

//!code-start: handler
/**
 * like sax_handler, sax_ns_handler provides empty defaults so we override only
 * the callbacks of interest; the element and attribute structs carry a resolved
 * namespace identifier in addition to the alias used in the source
 */
class sax_ns_parser_handler : public sax_ns_handler
{
public:
    void start_element(const sax_ns_parser_element& elem)
    {
        std::cout << "start element: " << elem.name;
        if (elem.ns)
            // a non-null identifier points to the resolved namespace URI
            std::cout << " (ns: " << elem.ns << ")";
        std::cout << std::endl;
    }

    void end_element(const sax_ns_parser_element& elem)
    {
        std::cout << "end element: " << elem.name << std::endl;
    }

    // keep the base overload used for declaration/PI attributes visible
    using sax_ns_handler::attribute;

    void attribute(const sax_ns_parser_attribute& attr)
    {
        std::cout << "  attribute: " << attr.name << "='" << attr.value << "'";
        if (attr.ns)
            std::cout << " (ns: " << attr.ns << ")";
        std::cout << std::endl;
    }

    void namespace_declaration(std::string_view alias, xmlns_id_t ns_id)
    {
        std::cout << "namespace declaration: alias='" << alias << "' uri='" << ns_id << "'" << std::endl;
    }
};
//!code-end: handler

int main()
{
    //!code-start: content
    std::string_view content =
        "<?xml version=\"1.0\"?>"
        "<list xmlns=\"http://example.com/default\" xmlns:x=\"http://example.com/extra\">"
        "<item x:rank=\"1\"/>"
        "<item x:rank=\"2\"/>"
        "</list>";
    //!code-end: content

    //!code-start: context
    // a context tracks the prefix-to-URI bindings as parsing descends through
    // element scopes; create a fresh one from the repository per stream
    xmlns_repository repo;
    xmlns_context cxt = repo.create_context();
    //!code-end: context

    //!code-start: parse
    sax_ns_parser_handler hdl;
    sax_ns_parser<sax_ns_parser_handler> parser(content, cxt, hdl);
    parser.parse();
    //!code-end: parse

    return EXIT_SUCCESS;
}
