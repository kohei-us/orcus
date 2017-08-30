
#include <orcus/json_document_tree.hpp>
#include <orcus/config.hpp>

#include <iostream>
#include <functional>
#include <vector>

void example_root_list()
{
    orcus::json::document_tree doc = {
        1.0, 2.0, "string value", false, nullptr
    };

    std::cout << doc.dump() << std::endl;
}

void example_list_nested()
{
    orcus::json::document_tree doc = {
        { true, false, nullptr },
        { 1.1, 2.2, "text" }
    };

    std::cout << doc.dump() << std::endl;
}

void example_list_object()
{
    orcus::json::document_tree doc = {
        { "key1", 1.2 },
        { "key2", "some text" },
    };

    std::cout << doc.dump() << std::endl;
}

void example_list_object_2()
{
    orcus::json::document_tree doc = {
        { "parent1", {
                { "child1", true  },
                { "child2", false },
                { "child3", 123.4 },
            }
        },
        { "parent2", "not-nested" },
    };

    std::cout << doc.dump() << std::endl;
}

void example_array_ambiguous()
{
    orcus::json::document_tree doc = {
        { "array", { "one", 987.0 } }
    };
}

void example_array_explicit()
{
    using namespace orcus;

    json::document_tree doc = {
        { "array", json::array({ "one", 987.0 }) }
    };

    std::cout << doc.dump() << std::endl;
}

void example_object_ambiguous()
{
    using namespace orcus;

    json::document_tree doc = {};

    try
    {
        auto root = doc.get_document_root();
    }
    catch (const json::document_error& e)
    {
        std::cout << e.what() << std::endl;
    }
}

void example_object_explicit_1()
{
    using namespace orcus;

    json::document_tree doc = json::object();

    std::cout << doc.dump() << std::endl;
}

void example_object_explicit_2()
{
    using namespace orcus;

    json::document_tree doc = {
        json::object(),
        json::object(),
        json::object()
    };

    std::cout << doc.dump() << std::endl;
}

void example_root_object_add_child()
{
    using namespace orcus;

    // Initialize the tree with an empty object.
    json::document_tree doc = json::object();

    // Get the root object, and assign three key-value pairs.
    json::node root = doc.get_document_root();
    root["child1"] = 1.0;
    root["child2"] = "string";
    root["child3"] = { true, false }; // implicit array

    // You can also create a key-value pair whose value is another object.
    root["child object"] = {
        { "key1", 100.0 },
        { "key2", 200.0 }
    };

    root["child array"] = json::array({ 1.1, 1.2, true }); // explicit array

    std::cout << doc.dump() << std::endl;
}

void example_root_array_add_child()
{
    using namespace orcus;

    // Initialize the tree with an empty array root.
    json::document_tree doc = json::array();

    // Get the root array.
    json::node root = doc.get_document_root();

    // Append values to the array.
    root.push_back(-1.2);
    root.push_back("string");
    root.push_back(true);
    root.push_back(nullptr);

    // You can append an object to the array via push_back() as well.
    root.push_back({{"key1", 1.1}, {"key2", 1.2}});

    std::cout << doc.dump() << std::endl;
}

int main()
{
    using func_type = std::function<void()>;

    std::vector<func_type> funcs = {
        example_root_list,
        example_list_nested,
        example_list_object,
        example_list_object_2,
        example_array_explicit,
        example_object_ambiguous,
        example_object_explicit_1,
        example_object_explicit_2,
        example_root_object_add_child,
        example_root_array_add_child,
    };

    for (func_type f : funcs)
    {
        std::cout << "--" << std::endl;
        f();
    }

    std::vector<func_type> funcs_exc = {
        example_array_ambiguous,
    };

    for (func_type f : funcs_exc)
    {
        try
        {
            f();
        }
        catch (orcus::json::key_value_error&)
        {
            // expected
        }
    }

    return EXIT_SUCCESS;
}
