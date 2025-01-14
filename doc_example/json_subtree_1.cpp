
//!code-start: headers
#include <orcus/json_document_tree.hpp>
#include <orcus/config.hpp>

#include <iostream>
#include <string_view>
//!code-end: headers

//!code-start: input
constexpr std::string_view input_json = R"(
{
  "id": "12345",
  "name": "John Doe",
  "email": "johndoe@example.com",
  "roles": ["admin", "editor"],
  "isActive": true,
  "profile": {
    "age": 34,
    "gender": "male",
    "address": {
      "street": "123 Elm Street",
      "city": "Springfield",
      "state": "IL",
      "zipCode": "62704"
    },
    "phoneNumbers": [
      {
        "type": "home",
        "number": "555-1234"
      },
      {
        "type": "work",
        "number": "555-5678"
      }
    ]
  },
  "preferences": {
    "notifications": {
      "email": true,
      "sms": false,
      "push": true
    },
    "theme": "dark",
    "language": "en-US"
  },
  "lastLogin": "2024-11-25T13:45:30Z",
  "purchaseHistory": [
    {
      "orderId": "A1001",
      "date": "2024-01-15T10:00:00Z",
      "total": 249.99,
      "items": [
        {
          "productId": "P123",
          "name": "Wireless Mouse",
          "quantity": 1,
          "price": 49.99
        },
        {
          "productId": "P124",
          "name": "Mechanical Keyboard",
          "quantity": 1,
          "price": 200.00
        }
      ]
    },
    {
      "orderId": "A1002",
      "date": "2024-06-10T14:20:00Z",
      "total": 119.99,
      "items": [
        {
          "productId": "P125",
          "name": "Noise Cancelling Headphones",
          "quantity": 1,
          "price": 119.99
        }
      ]
    }
  ]
}
)";
//!code-end: input

void subtree_1(const orcus::json::document_tree& doc)
{
    //!code-start: subtree 1
    orcus::json::subtree sub(doc, "$.profile.address");
    std::cout << sub.dump(2) << std::endl;
    //!code-end: subtree 1
}

void subtree_2(const orcus::json::document_tree& doc)
{
    //!code-start: subtree 2
    orcus::json::subtree sub(doc, "$.purchaseHistory[1].items[0]");
    std::cout << sub.dump(2) << std::endl;
    //!code-end: subtree 2
}

void subtree_3(const orcus::json::document_tree& doc)
{
    //!code-start: subtree 3
    orcus::json::subtree sub(doc, "$.purchaseHistory[*].items");
    std::cout << sub.dump(2) << std::endl;
    //!code-end: subtree 3
}

int main(int argc, char** argv)
{
    //!code-start: load doc
    orcus::json::document_tree doc;
    doc.load(input_json, orcus::json_config{});
    //!code-end: load doc

    std::cout << std::endl;
    subtree_1(doc);

    std::cout << std::endl;
    subtree_2(doc);

    std::cout << std::endl;
    subtree_3(doc);

    return EXIT_SUCCESS;
}
