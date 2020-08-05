
#include <orcus/json_parser.hpp>
#include <orcus/pstring.hpp>
#include <cstring>
#include <iostream>

using namespace std;

class json_parser_handler : public orcus::json_handler
{
public:
    void object_key(const char* p, size_t len, bool transient)
    {
        cout << "object key: " << orcus::pstring(p, len) << endl;
    }

    void string(const char* p, size_t len, bool transient)
    {
        cout << "string: " << orcus::pstring(p, len) << endl;
    }

    void number(double val)
    {
        cout << "number: " << val << endl;
    }
};

int main()
{
    const char* test_code = "{\"key1\": [1,2,3,4,5], \"key2\": 12.3}";
    size_t n_test_code = strlen(test_code);

    cout << "JSON string: " << test_code << endl;

    // Instantiate the parser with an own handler.
    json_parser_handler hdl;
    orcus::json_parser<json_parser_handler> parser(test_code, n_test_code, hdl);

    // Parse the string.
    parser.parse();

    return EXIT_SUCCESS;
}
