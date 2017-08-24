
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
        { "parent1",
            {
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

void example_object_explicit_1()
{
    using namespace orcus;

    json::document_tree doc = json::object();

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
        example_object_explicit_1,
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
