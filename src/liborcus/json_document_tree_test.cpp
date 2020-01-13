/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "orcus/stream.hpp"
#include "orcus/json_document_tree.hpp"
#include "orcus/json_parser_base.hpp"
#include "orcus/global.hpp"
#include "orcus/config.hpp"
#include "orcus/xml_namespace.hpp"
#include "orcus/dom_tree.hpp"

#include <cassert>
#include <cstdlib>
#include <iostream>
#include <cmath>

using namespace std;
using namespace orcus;

const char* json_test_dirs[] = {
    SRCDIR"/test/json/basic1/",
    SRCDIR"/test/json/basic2/",
    SRCDIR"/test/json/basic3/",
    SRCDIR"/test/json/basic4/",
    SRCDIR"/test/json/empty-array-1/",
    SRCDIR"/test/json/empty-array-2/",
    SRCDIR"/test/json/empty-array-3/",
    SRCDIR"/test/json/nested1/",
    SRCDIR"/test/json/nested2/",
    SRCDIR"/test/json/swagger/"
};

const char* json_test_refs_dirs[] = {
    SRCDIR"/test/json/refs1/",
};

bool string_expected(const json::const_node& node, const char* expected)
{
    if (node.type() != json::node_t::string)
        return false;

    if (node.string_value() == expected)
        return true;

    cerr << "expected='" << expected << "', actual='" << node.string_value() << "'" << endl;
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

    cerr << "expected=" << expected << ", actual=" << actual << endl;
    return false;
}

string dump_check_content(const json::document_tree& doc)
{
    string xml_strm = doc.dump_xml();
    assert(!xml_strm.empty());

    xmlns_repository repo;
    xmlns_context cxt = repo.create_context();
    dom::document_tree dom(cxt);
    dom.load(xml_strm);

    ostringstream os;
    dom.dump_compact(os);
    return os.str();
}

bool compare_check_contents(const file_content& expected, const std::string& actual)
{
    pstring _expected(expected.data(), expected.size());
    pstring _actual(actual.data(), actual.size());
    _expected = _expected.trim();
    _actual = _actual.trim();

    if (_expected != _actual)
    {
        size_t pos = locate_first_different_char(_expected, _actual);
        cout << create_parse_error_output(_expected, pos) << endl;
        cout << create_parse_error_output(_actual, pos) << endl;
    }

    return _expected == _actual;
}

void verify_input(json_config& test_config, const char* basedir)
{
    string json_file(basedir);
    json_file += "input.json";
    test_config.input_path = json_file;

    cout << "Testing " << json_file << endl;

    file_content content(json_file.data());
    json::document_tree doc;
    doc.load(content.data(), content.size(), test_config);

    string check_file(basedir);
    check_file += "check.txt";
    file_content check_master(check_file.data());
    string check_doc = dump_check_content(doc);

    bool result = compare_check_contents(check_master, check_doc);
    assert(result);
}

void test_json_parse()
{
    json_config test_config;

    for (size_t i = 0; i < ORCUS_N_ELEMENTS(json_test_dirs); ++i)
    {
        const char* basedir = json_test_dirs[i];
        verify_input(test_config, basedir);
    }
}

void test_json_resolve_refs()
{
    json_config test_config;
    test_config.resolve_references = true;

    for (size_t i = 0; i < ORCUS_N_ELEMENTS(json_test_refs_dirs); ++i)
    {
        const char* basedir = json_test_refs_dirs[i];
        verify_input(test_config, basedir);
    }
}

void test_json_parse_empty()
{
    json_config test_config;

    const char* tests[] = {
        "{}",
        "[]",
        "{\"key1\": {}, \"key2\": {}}"
    };

    for (size_t i = 0; i < ORCUS_N_ELEMENTS(tests); ++i)
    {
        const char* test = tests[i];
        cout << "JSON stream: '" << test << "' (" << strlen(test) << ")" << endl;
        json::document_tree doc;
        try
        {
            doc.load(test, strlen(test), test_config);
        }
        catch (const json::parse_error& e)
        {
            cout << create_parse_error_output(test, e.offset()) << endl;
            cout << e.what() << endl;
            assert(false);
        }
    }
}

void test_json_parse_invalid()
{
    json_config test_config;

    const char* invalids[] = {
        "[foo]",
        "[qwerty]",
        "[1,2] null",
        "{\"key\" 1: 12}",
        "[1,,2]",
        "\"key\": {\"inner\": 12}"
    };

    for (size_t i = 0; i < ORCUS_N_ELEMENTS(invalids); ++i)
    {
        const char* invalid_json = invalids[i];
        json::document_tree doc;
        try
        {
            doc.load(string(invalid_json, strlen(invalid_json)), test_config);
            cerr << "Invalid JSON expression is parsed as valid: '" << invalid_json << "'" << endl;
            assert(false);
        }
        catch (const json::parse_error& e)
        {
            // works as expected.
            cout << "invalid expression tested: " << invalid_json << endl;
            cout << "error message received: " << e.what() << endl;
        }
    }
}

std::unique_ptr<json::document_tree> get_doc_tree(const char* filepath)
{
    json_config test_config;

    cout << filepath << endl;
    file_content content(filepath);
    cout << "--- original" << endl;
    cout << content.str() << endl;

    auto doc = orcus::make_unique<json::document_tree>();
    doc->load(content.data(), content.size(), test_config);

    return doc;
}

void dump_and_load(
    const json::document_tree& doc, const std::function<void(json::const_node)>& test_func)
{
    json::document_tree doc2;
    std::string dumped = doc.dump();
    cout << "--- dumped" << endl;
    cout << dumped << endl;
    doc2.load(dumped, json_config());
    json::const_node node = doc2.get_document_root();
    test_func(node);
}

void test_json_traverse_basic1()
{
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
    auto test_func = [](json::const_node node)
    {
        assert(node.type() == json::node_t::array);
        assert(node.child_count() == 14);

        assert(string_expected(node.child(0), "I am string"));
        assert(string_expected(node.child(1), "me too"));
        assert(string_expected(node.child(2), ""));
        assert(string_expected(node.child(3), "\\"));
        assert(string_expected(node.child(4), "/"));
        assert(string_expected(node.child(5), "\\b"));
        assert(string_expected(node.child(6), "\\f"));
        assert(string_expected(node.child(7), "\\n"));
        assert(string_expected(node.child(8), "\\r"));
        assert(string_expected(node.child(9), "\\t"));
        assert(string_expected(node.child(10), "\"quoted\""));
        assert(string_expected(node.child(11), "http://www.google.com"));
        assert(string_expected(node.child(12), "one \\n two \\n three"));
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
    auto test_func = [](json::const_node node)
    {
        assert(node.type() == json::node_t::object);
        auto keys = node.keys();
        assert(keys.size() == 3);
        for (auto it = keys.begin(), ite = keys.end(); it != ite; ++it)
        {
            const pstring& key = *it;
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
    json::document_tree doc = json::object();
    json::node root = doc.get_document_root();

    /* {"test": [1.2, 1.3]} */
    auto node = root["test"];
    node = json::array();
    node.push_back(1.2);
    node.push_back(1.3);

    // Dump the doc as a string and reload it.
    doc.load(doc.dump(), json_config());
    root = doc.get_document_root();
    assert(root.type() == json::node_t::object);
    node = root["test"];
    assert(node.type() == json::node_t::array);
    assert(node.child_count() == 2u);
    assert(node.child(0).numeric_value() == 1.2);
    assert(node.child(1).numeric_value() == 1.3);
}

int main()
{
    try
    {
        test_json_parse();
        test_json_resolve_refs();
        test_json_parse_empty();
        test_json_parse_invalid();
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
    }
    catch (const orcus::general_error& e)
    {
        cerr << e.what() << endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
