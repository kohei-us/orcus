
//!code-start: headers
#include <orcus/sax_token_parser.hpp>
#include <orcus/tokens.hpp>
#include <orcus/xml_namespace.hpp>

#include <iostream>
//!code-end: headers

using namespace orcus;

//!code-start: tokens
/**
 * the token vocabulary maps names to integer tokens by position; index 0 is
 * reserved for XML_UNKNOWN_TOKEN, so the first real name starts at index 1
 */
const char* token_names[] = {
    "??",       // 0 - reserved for unknown names
    "catalog",  // 1
    "book",     // 2
    "id",       // 3
};

const xml_token_t XML_catalog = 1;
const xml_token_t XML_book = 2;
const xml_token_t XML_id = 3;
//!code-end: tokens

//!code-start: handler
class sax_token_parser_handler : public sax_token_handler
{
public:
    /**
     * names known to the vocabulary arrive as integer tokens, which can be
     * dispatched on directly instead of comparing strings
     */
    void start_element(const xml_token_element_t& elem)
    {
        std::cout << "start element: " << elem.raw_name << " (token=" << elem.name << ")";

        switch (elem.name)
        {
            case XML_catalog:
                std::cout << " -> recognized as catalog";
                break;
            case XML_book:
                std::cout << " -> recognized as book";
                break;
            default:
                std::cout << " -> unknown";
        }
        std::cout << std::endl;

        for (const xml_token_attr_t& attr : elem.attrs)
        {
            std::cout << "  attribute: " << attr.raw_name << " (token=" << attr.name
                << ") = '" << attr.value << "'";

            if (attr.name == XML_id)
                std::cout << " -> recognized as id";

            std::cout << std::endl;
        }
    }
};
//!code-end: handler

int main()
{
    //!code-start: content
    std::string_view content =
        "<?xml version=\"1.0\"?>"
        "<catalog>"
        "<book id=\"b1\"/>"
        "<book id=\"b2\"/>"
        "<magazine id=\"m1\"/>"
        "</catalog>";
    //!code-end: content

    //!code-start: setup
    // the token store is typically a global constant shared across parses
    tokens token_map(token_names, std::size(token_names));

    xmlns_repository repo;
    xmlns_context cxt = repo.create_context();
    //!code-end: setup

    //!code-start: parse
    sax_token_parser_handler hdl;
    sax_token_parser<sax_token_parser_handler> parser(content, token_map, cxt, hdl);
    parser.parse();
    //!code-end: parse

    return EXIT_SUCCESS;
}
