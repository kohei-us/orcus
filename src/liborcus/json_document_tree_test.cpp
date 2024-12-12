/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "test_global.hpp"

#include <orcus/stream.hpp>
#include <orcus/json_document_tree.hpp>
#include <orcus/json_parser_base.hpp>
#include <orcus/config.hpp>
#include <orcus/xml_namespace.hpp>
#include <orcus/dom_tree.hpp>
#include <orcus/measurement.hpp>

#include "filesystem_env.hpp"

#include <cassert>
#include <cstdlib>
#include <iostream>
#include <cmath>
#include <cstring>
#include <unordered_set>
#include <algorithm>

using namespace orcus;

const fs::path json_test_dirs[] = {
    SRCDIR"/test/json/basic1",
    SRCDIR"/test/json/basic2",
    SRCDIR"/test/json/basic3",
    SRCDIR"/test/json/basic4",
    SRCDIR"/test/json/empty-array-1",
    SRCDIR"/test/json/empty-array-2",
    SRCDIR"/test/json/empty-array-3",
    SRCDIR"/test/json/nested1",
    SRCDIR"/test/json/nested2",
    SRCDIR"/test/json/swagger",
    SRCDIR"/test/json/to-yaml-1",
    SRCDIR"/test/json/escape-control/printable",
    SRCDIR"/test/json/escape-control/not-printable",
    SRCDIR"/test/json/escape-surrogate/one-char",
    SRCDIR"/test/json/escape-surrogate/one-char-nbsp",
    SRCDIR"/test/json/escape-surrogate/one-char-with-ba",
    SRCDIR"/test/json/escape-surrogate/two-chars",
    SRCDIR"/test/json/escape-surrogate/two-chars-with-bc",
    SRCDIR"/test/json/escape-surrogate/mix",
};

const fs::path json_test_refs_dirs[] = {
    SRCDIR"/test/json/refs1",
};

bool string_expected(const json::const_node& node, const char* expected)
{
    if (node.type() != json::node_t::string)
        return false;

    if (node.string_value() == expected)
        return true;

    std::cerr << "expected='" << expected << "', actual='" << node.string_value() << "'" << std::endl;
    return false;
}

bool number_expected(
    const json::const_node& node, double expected,
    double decimal = 0.0, double exponent = 0.0)
{
    if (node.type() != json::node_t::number)
        return false;

    double actual = node.numeric_value();
    if (!decimal || !exponent)
        return actual == expected;

    // Remove the exponent component.
    actual /= std::pow(10.0, exponent);
    expected /= std::pow(10.0, exponent);

    // Only compare down to the specified decimal place.
    actual *= std::pow(10.0, decimal);
    expected *= std::pow(10.0, decimal);

    actual = std::round(actual);
    expected = std::round(expected);

    if (actual == expected)
        return true;

    std::cerr << "expected=" << expected << ", actual=" << actual << std::endl;
    return false;
}

std::string dump_check_content(const json::document_tree& doc)
{
    std::string xml_strm = doc.dump_xml();
    assert(!xml_strm.empty());

    xmlns_repository repo;
    xmlns_context cxt = repo.create_context();
    dom::document_tree dom(cxt);
    dom.load(xml_strm);

    std::ostringstream os;
    dom.dump_compact(os);
    return os.str();
}

bool compare_check_contents(const file_content& expected, std::string_view actual)
{
    std::string_view _expected(expected.data(), expected.size());
    _expected = trim(_expected);
    actual = trim(actual);

    if (_expected != actual)
    {
        std::size_t pos = locate_first_different_char(_expected, actual);
        std::cout << create_parse_error_output(_expected, pos) << std::endl;
        std::cout << create_parse_error_output(actual, pos) << std::endl;
    }

    return _expected == actual;
}

void verify_input(json_config& test_config, const fs::path& basedir)
{
    std::cout << "---" << std::endl;
    std::cout << "test directory: " << basedir << std::endl;

    fs::path json_file = basedir / "input.json";
    test_config.input_path = json_file.string();

    std::cout << "input: " << json_file << std::endl;

    file_content content(json_file.string());
    json::document_tree doc;
    doc.load(content.str(), test_config);

    if (fs::path check_file = basedir / "check.txt"; fs::is_regular_file(check_file))
    {
        std::cout << "check: " << check_file << std::endl;

        file_content check_master(check_file.string());
        std::string check_doc = dump_check_content(doc);

        bool result = compare_check_contents(check_master, check_doc);
        assert(result);
    }

    if (fs::path outpath = basedir / "output.json"; fs::is_regular_file(outpath))
    {
        // Test the json output.
        std::cout << "json output: " << outpath << std::endl;

        file_content expected(outpath.string());
        std::string actual = doc.dump(4);

        test::verify_content(__FILE__, __LINE__, expected.str(), actual);
    }

    if (fs::path outpath = basedir / "output.yaml"; fs::is_regular_file(outpath))
    {
        // Test the yaml output.
        std::cout << "yaml output: " << outpath << std::endl;

        file_content expected(outpath.string());
        std::string actual = doc.dump_yaml();

        test::verify_content(__FILE__, __LINE__, expected.str(), actual);
    }
}

void test_json_parse()
{
    ORCUS_TEST_FUNC_SCOPE;

    json_config test_config;

    for (const auto& basedir : json_test_dirs)
        verify_input(test_config, basedir);
}

void test_json_dump_indent_0()
{
    ORCUS_TEST_FUNC_SCOPE;

    json_config conf;
    json::document_tree doc;

    {
        file_content content(SRCDIR"/test/json/basic1/input.json");
        doc.load(content.str(), conf);

        auto dumped = doc.dump(0);
        assert(dumped == "[true, false, null]");
    }

    {
        file_content content(SRCDIR"/test/json/basic4/input.json");
        doc.load(content.str(), conf);

        auto dumped = doc.dump(0);
        assert(dumped == R"({"int": 12, "float": 0.125, "string": "blah..."})");
    }
}

void test_json_resolve_refs()
{
    ORCUS_TEST_FUNC_SCOPE;

    json_config test_config;
    test_config.resolve_references = true;

    for (size_t i = 0; i < std::size(json_test_refs_dirs); ++i)
    {
        fs::path basedir = json_test_refs_dirs[i];
        verify_input(test_config, basedir);
    }
}

void test_json_parse_empty()
{
    ORCUS_TEST_FUNC_SCOPE;

    json_config test_config;

    const char* tests[] = {
        "{}",
        "[]",
        "{\"key1\": {}, \"key2\": {}}"
    };

    for (size_t i = 0; i < std::size(tests); ++i)
    {
        const char* test = tests[i];
        std::cout << "JSON stream: '" << test << "' (" << std::strlen(test) << ")" << std::endl;
        json::document_tree doc;
        try
        {
            doc.load(test, test_config);
        }
        catch (const parse_error& e)
        {
            std::cout << create_parse_error_output(test, e.offset()) << std::endl;
            std::cout << e.what() << std::endl;
            assert(false);
        }
    }
}

void test_json_parse_invalid()
{
    ORCUS_TEST_FUNC_SCOPE;

    json_config test_config;

    const char* invalids[] = {
        "[foo]",
        "[qwerty]",
        "[1,2] null",
        "{\"key\" 1: 12}",
        "[1,,2]",
        "\"key\": {\"inner\": 12}"
    };

    for (std::size_t i = 0; i < std::size(invalids); ++i)
    {
        const char* invalid_json = invalids[i];
        json::document_tree doc;
        try
        {
            doc.load(invalid_json, test_config);
            std::cerr << "Invalid JSON expression is parsed as valid: '" << invalid_json << "'" << std::endl;
            assert(false);
        }
        catch (const parse_error& e)
        {
            // works as expected.
            std::cout << "invalid expression tested: " << invalid_json << std::endl;
            std::cout << "error message received: " << e.what() << std::endl;
        }
    }
}

std::unique_ptr<json::document_tree> get_doc_tree(const char* filepath)
{
    json_config test_config;

    std::cout << filepath << std::endl;
    file_content content(filepath);
    std::cout << "--- original" << std::endl;
    std::cout << content.str() << std::endl;

    auto doc = std::make_unique<json::document_tree>();
    doc->load(content.str(), test_config);

    return doc;
}

void dump_and_load(
    const json::document_tree& doc, const std::function<void(json::const_node)>& test_func)
{
    json::document_tree doc2;
    std::string dumped = doc.dump(4);
    std::cout << "--- dumped" << std::endl;
    std::cout << dumped << std::endl;
    doc2.load(dumped, json_config());
    json::const_node node = doc2.get_document_root();
    test_func(node);
}

void test_json_const_node_unset()
{
    ORCUS_TEST_FUNC_SCOPE;

    json::const_node node; // default constructor
    assert(node.type() == json::node_t::unset);
}

void test_json_traverse_basic1()
{
    ORCUS_TEST_FUNC_SCOPE;

    auto test_func = [](json::const_node node)
    {
        assert(node.type() == json::node_t::array);
        assert(node.child_count() == 3);
        assert(node.child(0).type() == json::node_t::boolean_true);
        assert(node.child(1).type() == json::node_t::boolean_false);
        assert(node.child(2).type() == json::node_t::null);

        // Move to child node and move back.
        json::const_node node2 = node.child(0).parent();
        assert(node.identity() == node2.identity());
    };

    const char* filepath = SRCDIR"/test/json/basic1/input.json";
    std::unique_ptr<json::document_tree> doc = get_doc_tree(filepath);
    json::const_node node = doc->get_document_root();
    test_func(node);
    dump_and_load(*doc, test_func);
}

void test_json_traverse_basic2()
{
    ORCUS_TEST_FUNC_SCOPE;

    auto test_func = [](json::const_node node)
    {
        assert(node.type() == json::node_t::array);
        assert(node.child_count() == 14);

        assert(string_expected(node.child(0), "I am string"));
        assert(string_expected(node.child(1), "me too"));
        assert(string_expected(node.child(2), ""));
        assert(string_expected(node.child(3), "\\"));
        assert(string_expected(node.child(4), "/"));
        assert(string_expected(node.child(5), "\b"));
        assert(string_expected(node.child(6), "\f"));
        assert(string_expected(node.child(7), "\n"));
        assert(string_expected(node.child(8), "\r"));
        assert(string_expected(node.child(9), "\t"));
        assert(string_expected(node.child(10), "\"quoted\""));
        assert(string_expected(node.child(11), "http://www.google.com"));
        assert(string_expected(node.child(12), "one \n two \n three"));
        assert(string_expected(node.child(13), "front segment 'single quote' and \"double quote\" end segment"));
    };

    const char* filepath = SRCDIR"/test/json/basic2/input.json";
    std::unique_ptr<json::document_tree> doc = get_doc_tree(filepath);
    json::const_node node = doc->get_document_root();
    test_func(node);
    dump_and_load(*doc, test_func);
}

void test_json_traverse_basic3()
{
    ORCUS_TEST_FUNC_SCOPE;

    auto test_func = [](json::const_node node)
    {
        assert(node.type() == json::node_t::array);
        assert(node.child_count() == 9);

        assert(number_expected(node.child(0), 0.0));
        assert(number_expected(node.child(1), 1.0));
        assert(number_expected(node.child(2), 2.0));
        assert(number_expected(node.child(3), 15.0));
        assert(number_expected(node.child(4), 12.34));
        assert(number_expected(node.child(5), -0.12));
        assert(number_expected(node.child(6), 1.2e+22, 1.0, 22.0));
        assert(number_expected(node.child(7), 1.11e-7, 2.0, -7.0));
        assert(number_expected(node.child(8), 11E2));
    };

    const char* filepath = SRCDIR"/test/json/basic3/input.json";
    std::unique_ptr<json::document_tree> doc = get_doc_tree(filepath);
    json::const_node node = doc->get_document_root();
    test_func(node);
    dump_and_load(*doc, test_func);
}

void test_json_traverse_basic4()
{
    ORCUS_TEST_FUNC_SCOPE;

    auto test_func = [](json::const_node node)
    {
        assert(node.type() == json::node_t::object);
        auto keys = node.keys();
        assert(keys.size() == 3);
        for (auto it = keys.begin(), ite = keys.end(); it != ite; ++it)
        {
            std::string_view key = *it;
            json::const_node child = node.child(key);
            if (key == "int")
                assert(number_expected(child, 12.0));
            else if (key == "float")
                assert(number_expected(child, 0.125));
            else if (key == "string")
                assert(string_expected(child, "blah..."));
            else
                assert(!"unexpected key");
        }
    };

    const char* filepath = SRCDIR"/test/json/basic4/input.json";
    std::unique_ptr<json::document_tree> doc = get_doc_tree(filepath);
    json::const_node node = doc->get_document_root();
    test_func(node);
    dump_and_load(*doc, test_func);
}

void test_json_traverse_nested1()
{
    ORCUS_TEST_FUNC_SCOPE;

    auto test_func = [](json::const_node node)
    {
        uintptr_t root_id = node.identity();

        assert(node.type() == json::node_t::object);
        assert(node.child_count() == 1);

        node = node.child(0);
        assert(node.type() == json::node_t::array);
        assert(node.child_count() == 3);

        assert(number_expected(node.child(0), 1.0));
        assert(number_expected(node.child(1), 2.0));
        assert(number_expected(node.child(2), 3.0));

        node = node.parent();
        assert(node.identity() == root_id);
    };

    const char* filepath = SRCDIR"/test/json/nested1/input.json";
    std::unique_ptr<json::document_tree> doc = get_doc_tree(filepath);
    json::const_node node = doc->get_document_root();
    test_func(node);
    dump_and_load(*doc, test_func);
}

void test_json_traverse_nested2()
{
    ORCUS_TEST_FUNC_SCOPE;

    auto test_func = [](json::const_node node)
    {
        assert(node.type() == json::node_t::array);
        assert(node.child_count() == 3);

        node = node.child(0);
        assert(node.type() == json::node_t::object);
        assert(number_expected(node.child("value"), 1.0));
        node = node.parent();

        node = node.child(1);
        assert(node.type() == json::node_t::object);
        assert(number_expected(node.child("value"), 2.0));
        node = node.parent();

        node = node.child(2);
        assert(node.type() == json::node_t::object);
        assert(number_expected(node.child("value"), 3.0));
        node = node.parent();
    };

    const char* filepath = SRCDIR"/test/json/nested2/input.json";
    std::unique_ptr<json::document_tree> doc = get_doc_tree(filepath);
    json::const_node node = doc->get_document_root();
    test_func(node);
    dump_and_load(*doc, test_func);
}

void test_json_init_list_flat1()
{
    ORCUS_TEST_FUNC_SCOPE;

    json::document_tree doc = { 1.0, 2.0, 3.0, 4.0 };
    json::const_node node = doc.get_document_root();
    assert(node.type() == json::node_t::array);
    assert(node.child_count() == 4);

    node = node.child(0);
    assert(node.type() == json::node_t::number);
    assert(node.numeric_value() == 1.0);
    node = node.parent();

    node = node.child(1);
    assert(node.type() == json::node_t::number);
    assert(node.numeric_value() == 2.0);
    node = node.parent();

    node = node.child(2);
    assert(node.type() == json::node_t::number);
    assert(node.numeric_value() == 3.0);
    node = node.parent();

    node = node.child(3);
    assert(node.type() == json::node_t::number);
    assert(node.numeric_value() == 4.0);
    node = node.parent();

    // Use iterators.
    auto it = node.begin();
    assert(it->type() == json::node_t::number);
    assert(it->numeric_value() == 1.0);
    ++it;
    assert(it->type() == json::node_t::number);
    assert(it->numeric_value() == 2.0);
    auto test = it++; // post increment
    assert(test->numeric_value() == 2.0);
    assert(it->type() == json::node_t::number);
    assert(it->numeric_value() == 3.0);
    test = ++it; // pre increment
    assert(test->numeric_value() == 4.0);
    ++it;
    assert(it == node.end());
    --it;
    assert(it->numeric_value() == 4.0);
    test = it--;
    assert(test->numeric_value() == 4.0);
    assert(it->numeric_value() == 3.0);

    doc = { nullptr };
    node = doc.get_document_root();
    assert(node.type() == json::node_t::array);
    assert(node.child_count() == 1);

    node = node.child(0);
    assert(node.type() == json::node_t::null);

    doc = { true, false };
    node = doc.get_document_root();
    assert(node.type() == json::node_t::array);
    assert(node.child_count() == 2);

    node = node.child(0);
    assert(node.type() == json::node_t::boolean_true);
    node = node.parent();

    node = node.child(1);
    assert(node.type() == json::node_t::boolean_false);
    node = node.parent();

    doc = { "A", "B", "C" };
    node = doc.get_document_root();
    assert(node.type() == json::node_t::array);
    assert(node.child_count() == 3);

    node = node.child(0);
    assert(node.type() == json::node_t::string);
    assert(node.string_value() == "A");
    node = node.parent();

    node = node.child(1);
    assert(node.type() == json::node_t::string);
    assert(node.string_value() == "B");
    node = node.parent();

    node = node.child(2);
    assert(node.type() == json::node_t::string);
    assert(node.string_value() == "C");
}

void test_json_init_list_nested1()
{
    ORCUS_TEST_FUNC_SCOPE;

    json::document_tree doc = {
        { true, false, nullptr },
        { 1.1, 2.2, "text" }
    };

    json::const_node node = doc.get_document_root();
    assert(node.type() == json::node_t::array);
    assert(node.child_count() == 2);

    node = node.child(0);
    assert(node.type() == json::node_t::array);
    assert(node.child_count() == 3);
    assert(node.child(0).type() == json::node_t::boolean_true);
    assert(node.child(1).type() == json::node_t::boolean_false);
    assert(node.child(2).type() == json::node_t::null);
    node = node.parent();

    node = node.child(1);
    assert(node.type() == json::node_t::array);
    assert(node.child_count() == 3);
    assert(node.child(0).type() == json::node_t::number);
    assert(node.child(0).numeric_value() == 1.1);
    assert(node.child(1).type() == json::node_t::number);
    assert(node.child(1).numeric_value() == 2.2);
    assert(node.child(2).type() == json::node_t::string);
    assert(node.child(2).string_value() == "text");
}

void test_json_init_list_object1()
{
    ORCUS_TEST_FUNC_SCOPE;

    json::document_tree doc = {
        { "key1", 1.2 },
        { "key2", "some text" },
    };

    json::const_node node = doc.get_document_root();
    assert(node.type() == json::node_t::object);
    assert(node.child_count() == 2);
    assert(node.key(0) == "key1");
    assert(node.key(1) == "key2");

    node = node.child("key1");
    assert(node.type() == json::node_t::number);
    assert(node.numeric_value() == 1.2);
    node = node.parent();

    node = node.child("key2");
    assert(node.type() == json::node_t::string);
    assert(node.string_value() == "some text");
}

void test_json_init_list_object2()
{
    ORCUS_TEST_FUNC_SCOPE;

    // nested objects.
    json::document_tree doc = {
        { "parent1",
            {
                { "child1", true  },
                { "child2", false },
                { "child3", 123.4 },
            }
        },
        { "parent2", "not-nested" },
    };

    json::const_node node = doc.get_document_root();
    assert(node.type() == json::node_t::object);
    assert(node.child_count() == 2);
    assert(node.key(0) == "parent1");
    assert(node.key(1) == "parent2");

    node = node.child("parent1");
    assert(node.type() == json::node_t::object);
    assert(node.child_count() == 3);
    assert(node.key(0) == "child1");
    assert(node.key(1) == "child2");
    assert(node.key(2) == "child3");

    assert(node.child("child1").type() == json::node_t::boolean_true);
    assert(node.child("child2").type() == json::node_t::boolean_false);
    assert(node.child("child3").type() == json::node_t::number);
    assert(node.child("child3").numeric_value() == 123.4);

    node = node.parent();

    node = node.child("parent2");
    assert(node.type() == json::node_t::string);
    assert(node.string_value() == "not-nested");
}

void test_json_init_list_explicit_array()
{
    ORCUS_TEST_FUNC_SCOPE;

    try
    {
        // This structure is too ambiguous and cannot be implicitly
        // determined.
        json::document_tree doc = {
            { "array", { "one", 987.0 } }
        };
        assert(!"key_value_error was not thrown");
    }
    catch (const json::key_value_error&)
    {
        // expected.
    }

    // Explicitly define an array instead.
    json::document_tree doc = {
        { "array", json::array({ "one", 987.0 }) }
    };

    json::node node = doc.get_document_root();
    assert(node.type() == json::node_t::object);
    assert(node.child_count() == 1);
    assert(node.key(0) == "array");

    node = node.child(0);
    assert(node.type() == json::node_t::array);
    assert(node.child_count() == 2);
    assert(node.child(0).string_value() == "one");
    assert(node.child(1).numeric_value() == 987.0);

    doc = json::array({1, 2, 3});
    node = doc.get_document_root();
    assert(node.type() == json::node_t::array);
    assert(node.child_count() == 3);
    assert(node.child(0).numeric_value() == 1.0);
    assert(node.child(1).numeric_value() == 2.0);
    assert(node.child(2).numeric_value() == 3.0);

    node.push_back(4);
    node.push_back(5);
    assert(node.child_count() == 5);
    assert(node.child(3).numeric_value() == 4.0);
    assert(node.child(4).numeric_value() == 5.0);

    // empty JSON with array root.
    json::document_tree doc2 = json::array();
    node = doc2.get_document_root();
    assert(node.type() == json::node_t::array);
    assert(node.child_count() == 0);

    // Assigning a non-const node to a const one should work.
    json::const_node cnode = node;
    assert(node.type() == json::node_t::array);
    assert(node.child_count() == 0);
}

void test_json_init_list_explicit_object()
{
    ORCUS_TEST_FUNC_SCOPE;

    json::document_tree doc = json::object();
    json::node node = doc.get_document_root();
    assert(node.type() == json::node_t::object);
    assert(node.child_count() == 0);

    // Initialize with an array of 3 empty objects.
    doc = {
        json::object(),
        json::object(),
        json::object()
    };

    node = doc.get_document_root();
    assert(node.type() == json::node_t::array);
    assert(node.child_count() == 3);

    for (size_t i = 0; i < 3; ++i)
    {
        node = node.child(i);
        assert(node.type() == json::node_t::object);
        assert(node.child_count() == 0);
        node = node.parent();
    }
}

void test_json_init_root_object_add_child()
{
    ORCUS_TEST_FUNC_SCOPE;

    json::document_tree doc = json::object();
    json::node node = doc.get_document_root();
    assert(node.type() == json::node_t::object);
    assert(node.child_count() == 0);

    node["child1"] = 1.0;

    assert(node.child_count() == 1);

    node = node.child("child1");
    assert(node.type() == json::node_t::number);
    assert(node.numeric_value() == 1.0);

    node = node.parent();
    node["child2"] = "foo";

    assert(node.child_count() == 2);

    node = node.child("child2");
    assert(node.type() == json::node_t::string);
    assert(node.string_value() == "foo");

    node = node.parent();

    // Access to child via [] operator.
    node = node["child1"];
    assert(node.type() == json::node_t::number);
    assert(node.numeric_value() == 1.0);

    node = node.parent();
    node["child3"] = { true, false };

    node = node.child("child3");
    assert(node.type() == json::node_t::array);
    assert(node.child_count() == 2);

    node = node.child(0);
    assert(node.type() == json::node_t::boolean_true);

    // Move up to the parent array.
    node = node.parent();
    assert(node.type() == json::node_t::array);
    assert(node.child_count() == 2);

    // Move down to the other child node.
    node = node.child(1);
    assert(node.type() == json::node_t::boolean_false);

    // Move up to the root node.
    node = node.parent().parent();
    assert(node.type() == json::node_t::object);
    assert(node.child_count() == 3);

    node["child1"] = true; // overwrite an existing node.
    node = node.child("child1");
    assert(node.type() == json::node_t::boolean_true);

    // direct assignment.
    node = false;
    assert(node.type() == json::node_t::boolean_false);

    node = node.parent().child("child1"); // make sure the link is still intact.
    assert(node.type() == json::node_t::boolean_false);

    node = node.parent();
    node["null-child"] = nullptr;

    node = node.child("null-child");
    assert(node.type() == json::node_t::null);

    node = node.parent();
    node["object-child"] = json::object();

    node = node.child("object-child");
    assert(node.type() == json::node_t::object);
    assert(node.child_count() == 0);

    node["array"] = json::array({true, false, nullptr});

    node = node.child("array");
    assert(node.type() == json::node_t::array);
    assert(node.child_count() == 3);

    node = node.parent();
    node["nested-object"] =
    {
        { "key1", "foo" },
        { "key2", 12.34 }
    };

    node = node.child("nested-object");
    assert(node.type() == json::node_t::object);
    assert(node.child_count() == 2);
    assert(node.child("key1").string_value() == "foo");
    assert(node.child("key2").numeric_value() == 12.34);
}

void test_json_init_empty_array()
{
    ORCUS_TEST_FUNC_SCOPE;

    json::document_tree doc = json::array();
    json::node node = doc.get_document_root();
    assert(node.type() == json::node_t::array);

    doc = {
        { "key1", json::array({true, false}) },
        { "key2", json::array() } // empty array
    };

    node = doc.get_document_root();
    assert(node.type() == json::node_t::object);
    node = node["key1"];
    assert(node.type() == json::node_t::array);
    node = node.parent()["key2"];
    assert(node.type() == json::node_t::array);
}

void test_json_dynamic_object_keys()
{
    ORCUS_TEST_FUNC_SCOPE;

    json::document_tree doc = json::object();
    json::node root = doc.get_document_root();

    /* {"test": [1.2, 1.3]} */
    auto node = root["test"];
    node = json::array();
    node.push_back(1.2);
    node.push_back(1.3);

    // Dump the doc as a string and reload it.
    doc.load(doc.dump(4), json_config());
    root = doc.get_document_root();
    assert(root.type() == json::node_t::object);
    node = root["test"];
    assert(node.type() == json::node_t::array);
    assert(node.child_count() == 2u);
    assert(node.child(0).numeric_value() == 1.2);
    assert(node.child(1).numeric_value() == 1.3);
}

void test_json_dump_subtree()
{
    ORCUS_TEST_FUNC_SCOPE;

    file_content input_json(SRCDIR"/test/json/subtree/medium1/input.json");

    json_config test_config;

    json::document_tree doc;
    doc.load(input_json.str(), test_config);

    auto node = doc.get_document_root();
    node = node.child("profile");

    {
        file_content expected(SRCDIR"/test/json/subtree/medium1/profile.i4.json");
        bool result = compare_check_contents(expected, node.dump(2));
        assert(result);
    }

    node = node.child("phoneNumbers");

    {
        file_content expected(SRCDIR"/test/json/subtree/medium1/profile-phoneNumbers.i3.json");
        bool result = compare_check_contents(expected, node.dump(3));
        assert(result);
    }

    node = node.child(0);

    {
        file_content expected(SRCDIR"/test/json/subtree/medium1/profile-phoneNumbers-0.i1.json");
        bool result = compare_check_contents(expected, node.dump(1));
        assert(result);
    }

    // check single values
    node = doc.get_document_root().child("isActive");
    assert(node.dump(0) == "true");

    node = doc.get_document_root().child("email");
    assert(node.dump(0) == R"("johndoe@example.com")"); // NB: note the double quotes!

    node = doc.get_document_root().child("profile").child("age");
    assert(node.dump(0) == "34");

    node = doc.get_document_root().child("preferences").child("notifications").child("sms");
    assert(node.dump(0) == "false");
}

void test_json_subtree()
{
    ORCUS_TEST_FUNC_SCOPE;

    json_config test_config;

    const fs::path test_dirs[] = {
        SRCDIR"/test/json/subtree/one-array",
        SRCDIR"/test/json/subtree/array-of-objects",
        SRCDIR"/test/json/subtree/medium1",
        SRCDIR"/test/json/subtree/medium2",
    };

    auto extract_spec = [&test_config](const fs::path& p)
    {
        file_content spec(p.string());
        json::document_tree spec_doc;
        spec_doc.load(spec.str(), test_config);

        auto root = spec_doc.get_document_root();
        std::string path{root.child("path").string_value()};
        int indent = root.child("indent").numeric_value();
        // constructing path from std::string_view is coming in later version of
        // boost. See
        // https://github.com/boostorg/filesystem/commit/b219d9fb8af760b54b014d1223ebd5d43b4d07b5
        auto output = p.parent_path() / std::string{root.child("output").string_value()};

        return std::make_tuple(path, indent, output);
    };

    for (const auto& dir : test_dirs)
    {
        std::vector<fs::path> path_output_specs;
        fs::path input;

        for (const auto& entry : fs::directory_iterator(dir))
        {
            if (!fs::is_regular_file(entry.path()))
                continue;

            auto filename = entry.path().filename().string();
            if (filename == "input.json")
                input = entry.path();
            else if (filename.find("output-spec") == 0)
                // filename starts with 'output-spec'
                path_output_specs.push_back(entry.path());
        }

        std::sort(path_output_specs.begin(), path_output_specs.end());
        assert(fs::is_regular_file(input));

        std::cout << "- input: " << input << std::endl;

        file_content input_json(input.string());

        json::document_tree doc;
        doc.load(input_json.str(), test_config);

        for (const auto& output_spec : path_output_specs)
        {
            auto [path, indent, output_path] = extract_spec(output_spec);
            file_content expected(output_path.string());

            std::cout
                << "  - output: " << output_path << "\n"
                << "    path: '" << path << "'\n"
                << "    indent: " << indent << std::endl;

            json::subtree st(doc, path);

            bool result = compare_check_contents(expected, st.dump(indent));
            assert(result);
            continue;
        }
    }
}

int main()
{
    try
    {
        test_json_parse();
        test_json_dump_indent_0();
        test_json_resolve_refs();
        test_json_parse_empty();
        test_json_parse_invalid();
        test_json_const_node_unset();
        test_json_traverse_basic1();
        test_json_traverse_basic2();
        test_json_traverse_basic3();
        test_json_traverse_basic4();
        test_json_traverse_nested1();
        test_json_traverse_nested2();

        test_json_init_list_flat1();
        test_json_init_list_nested1();
        test_json_init_list_object1();
        test_json_init_list_object2();
        test_json_init_list_explicit_array();
        test_json_init_list_explicit_object();
        test_json_init_root_object_add_child();
        test_json_init_empty_array();
        test_json_dynamic_object_keys();
        test_json_dump_subtree();

        test_json_subtree();
    }
    catch (const orcus::general_error& e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
