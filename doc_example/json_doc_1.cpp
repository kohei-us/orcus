
#include <orcus/json_document_tree.hpp>
#include <orcus/config.hpp>

#include <cstdlib>
#include <iostream>

using namespace std;

const char* json_string = "{"
"   \"name\": \"John Doe\","
"   \"occupation\": \"Software Engineer\","
"   \"score\": [89, 67, 90]"
"}";

int main()
{
    using node = orcus::json::node;

    orcus::json_config config; // Use default configuration.

    orcus::json::document_tree doc;
    doc.load(json_string, config);

    // Root is an object containing three key-value pairs.
    node root = doc.get_document_root();

    for (std::string_view key : root.keys())
    {
        node value = root.child(key);
        switch (value.type())
        {
            case orcus::json::node_t::string:
                // string value
                cout << key << ": " << value.string_value() << endl;
                break;
            case orcus::json::node_t::array:
            {
                // array value
                cout << key << ":" << endl;

                for (size_t i = 0; i < value.child_count(); ++i)
                {
                    node array_element = value.child(i);
                    cout << "  - " << array_element.numeric_value() << endl;
                }
                break;
            }
            default:
                ;
        }
    }

    return EXIT_SUCCESS;
}
