
//!code-start: headers
#include <orcus/sax_parser.hpp>

#include <iostream>
//!code-end: headers

//!code-start: handler
/**
 * the handler only needs to define the callbacks it cares about; inheriting
 * from orcus::sax_handler supplies empty defaults for the rest
 */
class sax_parser_handler : public orcus::sax_handler
{
public:
    void start_element(const orcus::sax::parser_element& elem)
    {
        std::cout << "start element: " << elem.name << std::endl;
    }

    void end_element(const orcus::sax::parser_element& elem)
    {
        std::cout << "end element: " << elem.name << std::endl;
    }

    void attribute(const orcus::sax::parser_attribute& attr)
    {
        std::cout << "  attribute: " << attr.name << "='" << attr.value << "'" << std::endl;
    }

    void characters(std::string_view val, bool /*transient*/)
    {
        // skip whitespace-only segments between elements
        if (val.find_first_not_of(" \t\r\n") == std::string_view::npos)
            return;

        std::cout << "  characters: " << val << std::endl;
    }
};
//!code-end: handler

int main()
{
    //!code-start: content
    std::string_view content =
        "<?xml version=\"1.0\"?>"
        "<catalog>"
        "<book id=\"b1\">Go</book>"
        "<book id=\"b2\">C++</book>"
        "</catalog>";
    //!code-end: content

    //!code-start: parse
    // instantiate the parser with the content and an own handler
    sax_parser_handler hdl;
    orcus::sax_parser<sax_parser_handler> parser(content, hdl);

    // parse the content
    parser.parse();
    //!code-end: parse

    return EXIT_SUCCESS;
}
