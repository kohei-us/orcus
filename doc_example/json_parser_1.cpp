
#include <orcus/json_parser.hpp>
#include <cstring>
#include <iostream>

using namespace std;

class json_parser_handler : public orcus::json_handler
{
public:
    void object_key(std::string_view key, bool /*transient*/)
    {
        cout << "object key: " << key << endl;
    }

    void string(std::string_view val, bool /*transient*/)
    {
        cout << "string: " << val << endl;
    }

    void number(double val)
    {
        cout << "number: " << val << endl;
    }
};

int main()
{
    const char* test_code = "{\"key1\": [1,2,3,4,5], \"key2\": 12.3}";

    cout << "JSON string: " << test_code << endl;

    // Instantiate the parser with an own handler.
    json_parser_handler hdl;
    orcus::json_parser<json_parser_handler> parser(test_code, hdl);

    // Parse the string.
    parser.parse();

    return EXIT_SUCCESS;
}
