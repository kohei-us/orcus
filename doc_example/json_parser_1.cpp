
#include <orcus/json_parser.hpp>
#include <cstring>
#include <iostream>

class json_parser_handler : public orcus::json_handler
{
public:
    void object_key(std::string_view key, bool /*transient*/)
    {
        std::cout << "object key: " << key << std::endl;
    }

    void string(std::string_view val, bool /*transient*/)
    {
        std::cout << "string: " << val << std::endl;
    }

    void number(double val)
    {
        std::cout << "number: " << val << std::endl;
    }
};

int main()
{
    const char* test_code = "{\"key1\": [1,2,3,4,5], \"key2\": 12.3}";

    std::cout << "JSON string: " << test_code << std::endl;

    // Instantiate the parser with an own handler.
    json_parser_handler hdl;
    orcus::json_parser<json_parser_handler> parser(test_code, hdl);

    // Parse the string.
    parser.parse();

    return EXIT_SUCCESS;
}
