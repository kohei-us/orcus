/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "orcus/yaml_document_tree.hpp"
#include "orcus/stream.hpp"
#include "orcus/pstring.hpp"
#include "orcus/global.hpp"
#include "orcus/yaml_parser_base.hpp"

#include <cassert>
#include <iostream>
#include <cmath>

#include <boost/filesystem.hpp>

using namespace orcus;
using namespace std;

namespace fs = boost::filesystem;

bool string_expected(const yaml_document_tree::node& node, const char* expected)
{
    if (node.type() != yaml_node_t::string)
        return false;

    if (node.string_value() == expected)
        return true;

    cerr << "expected='" << expected << "', actual='" << node.string_value() << "'" << endl;
    return false;
}

bool number_expected(
    const yaml_document_tree::node& node, double expected,
    double decimal = 0.0, double exponent = 0.0)
{
    if (node.type() != yaml_node_t::number)
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

    return actual == expected;
}

yaml_document_tree load_doc(const char* filepath)
{
    cout << filepath << endl;
    string strm = load_file_content(filepath);
    cout << strm << endl;
    yaml_document_tree doc;
    doc.load(strm);

    return doc;
}

void test_yaml_invalids()
{
    // Get all yaml files in this directory.
    fs::path dirpath(SRCDIR"/test/yaml/invalids/");
    fs::directory_iterator it_end;

    size_t file_count = 0;

    for (fs::directory_iterator it(dirpath); it != it_end; ++it)
    {
        auto path = it->path();
        if (!fs::is_regular_file(path))
            continue;

        if (fs::extension(path) != ".yaml")
            continue;

        ++file_count;

        string strm = load_file_content(path.string().data());
        yaml_document_tree doc;

        try
        {
            doc.load(strm);
            assert(!"yaml::parse_error was not thrown, but expected to be.");
        }
        catch (const yaml::parse_error&)
        {
            // This is expected.
        }
    }

    assert(file_count > 0);
}

void test_yaml_parse_basic1()
{
    yaml_document_tree doc = load_doc(SRCDIR"/test/yaml/basic1/input.yaml");

    assert(doc.get_document_count() == 1);

    // Document root is a map node with 4 elements.
    yaml_document_tree::node root = doc.get_document_root(0);
    uintptr_t root_id = root.identity();
    assert(root.type() == yaml_node_t::map);
    assert(root.child_count() == 4);

    std::vector<yaml_document_tree::node> keys = root.keys();
    assert(keys.size() == 4);

    yaml_document_tree::node key = keys[0];
    assert(key.type() == yaml_node_t::string);
    assert(key.string_value() == "dict");

    key = keys[1];
    assert(key.type() == yaml_node_t::string);
    assert(key.string_value() == "list");

    key = keys[2];
    assert(key.type() == yaml_node_t::string);
    assert(key.string_value() == "number");

    key = keys[3];
    assert(key.type() == yaml_node_t::string);
    assert(key.string_value() == "string");

    // first child is a map.
    yaml_document_tree::node node = root.child(keys[0]);
    assert(node.type() == yaml_node_t::map);
    assert(node.child_count() == 3);

    key = node.key(0);
    assert(key.type() == yaml_node_t::string);
    assert(key.string_value() == "a");

    key = node.key(1);
    assert(key.type() == yaml_node_t::string);
    assert(key.string_value() == "b");

    key = node.key(2);
    assert(key.type() == yaml_node_t::string);
    assert(key.string_value() == "c");

    node = node.child(0);
    assert(node.type() == yaml_node_t::number);
    assert(node.numeric_value() == 1.0);
    node = node.parent();

    node = node.child(1);
    assert(node.type() == yaml_node_t::number);
    assert(node.numeric_value() == 2.0);
    node = node.parent();

    node = node.child(2);
    assert(node.type() == yaml_node_t::sequence);
    assert(node.child_count() == 2);
    yaml_document_tree::node child = node.child(0);
    assert(child.type() == yaml_node_t::string);
    assert(child.string_value() == "foo");
    child = node.child(1);
    assert(child.type() == yaml_node_t::string);
    assert(child.string_value() == "bar");

    // Go up to the root node.
    node = node.parent().parent();
    assert(node.type() == yaml_node_t::map);
    assert(node.identity() == root_id);

    node = node.child(keys[1]);
    assert(node.type() == yaml_node_t::sequence);
    assert(node.child_count() == 3);

    node = node.child(0);
    assert(node.type() == yaml_node_t::number);
    assert(node.numeric_value() == 1.0);
    node = node.parent();

    node = node.child(1);
    assert(node.type() == yaml_node_t::number);
    assert(node.numeric_value() == 2.0);
    node = node.parent();

    node = node.child(2);
    assert(node.type() == yaml_node_t::map);
    assert(node.child_count() == 3);

    key = node.key(0);
    assert(key.type() == yaml_node_t::string);
    assert(key.string_value() == "a");

    key = node.key(1);
    assert(key.type() == yaml_node_t::string);
    assert(key.string_value() == "b");

    key = node.key(2);
    assert(key.type() == yaml_node_t::string);
    assert(key.string_value() == "c");

    node = node.child(0);
    assert(node.type() == yaml_node_t::number);
    assert(node.numeric_value() == 1.1);
    node = node.parent();

    node = node.child(1);
    assert(node.type() == yaml_node_t::number);
    assert(node.numeric_value() == 1.2);
    node = node.parent();

    node = node.child(2);
    assert(node.type() == yaml_node_t::number);
    assert(node.numeric_value() == 1.3);
    node = node.parent();

    node = node.parent().parent();  // back to the root.
    assert(node.identity() == root_id);

    key = node.key(2);
    assert(key.type() == yaml_node_t::string);
    assert(key.string_value() == "number");

    key = node.key(3);
    assert(key.type() == yaml_node_t::string);
    assert(key.string_value() == "string");

    node = node.child(2);
    assert(node.type() == yaml_node_t::number);
    assert(node.numeric_value() == 12.3);
    node = node.parent();

    node = node.child(3);
    assert(node.type() == yaml_node_t::string);
    assert(node.string_value() == "foo");
    node = node.parent();
}

void test_yaml_parse_basic2()
{
    yaml_document_tree doc = load_doc(SRCDIR"/test/yaml/basic2/input.yaml");

    assert(doc.get_document_count() == 1);
    yaml_document_tree::node node = doc.get_document_root(0);

    assert(node.type() == yaml_node_t::sequence);
    assert(node.child_count() == 3);

    node = node.child(0);
    assert(node.type() == yaml_node_t::number);
    assert(node.numeric_value() == 1);
    node = node.parent();

    node = node.child(1);
    assert(node.type() == yaml_node_t::number);
    assert(node.numeric_value() == 2);
    node = node.parent();

    node = node.child(2);
    assert(node.type() == yaml_node_t::number);
    assert(node.numeric_value() == 3);
    node = node.parent();
}

void test_yaml_parse_basic3()
{
    yaml_document_tree doc = load_doc(SRCDIR"/test/yaml/basic3/input.yaml");

    assert(doc.get_document_count() == 1);
    yaml_document_tree::node node = doc.get_document_root(0);

    assert(node.type() == yaml_node_t::sequence);
    assert(node.child_count() == 2);

    node = node.child(0);
    assert(node.type() == yaml_node_t::map);
    assert(node.child_count() == 3);

    assert(string_expected(node.key(0), "a"));
    assert(string_expected(node.key(1), "b"));
    assert(string_expected(node.key(2), "c"));

    assert(number_expected(node.child(0), 1));
    assert(number_expected(node.child(1), 2));
    assert(number_expected(node.child(2), 3));

    node = node.parent();

    node = node.child(1);
    assert(node.type() == yaml_node_t::map);
    assert(node.child_count() == 3);

    assert(string_expected(node.key(0), "d"));
    assert(string_expected(node.key(1), "e"));
    assert(string_expected(node.key(2), "f"));

    assert(number_expected(node.child(0), 4));
    assert(number_expected(node.child(1), 5));
    assert(number_expected(node.child(2), 6));
}

void test_yaml_parse_null()
{
    yaml_document_tree doc = load_doc(SRCDIR"/test/yaml/null/input.yaml");

    assert(doc.get_document_count() == 1);
    yaml_document_tree::node node = doc.get_document_root(0);

    assert(node.type() == yaml_node_t::sequence);
    assert(node.child_count() == 6);

    node = node.child(0);
    assert(node.type() == yaml_node_t::null);
    node = node.parent();

    node = node.child(1);
    assert(node.type() == yaml_node_t::null);
    node = node.parent();

    node = node.child(2);
    assert(node.type() == yaml_node_t::null);
    node = node.parent();

    node = node.child(3);
    assert(node.type() == yaml_node_t::null);
    node = node.parent();

    node = node.child(4);
    assert(node.type() == yaml_node_t::string);
    assert(node.string_value() == "nULL");
    node = node.parent();

    node = node.child(5);
    assert(node.type() == yaml_node_t::string);
    assert(node.string_value() == "NUll");
    node = node.parent();
}

void test_yaml_parse_boolean()
{
    yaml_document_tree doc = load_doc(SRCDIR"/test/yaml/boolean/input.yaml");

    assert(doc.get_document_count() == 1);
    yaml_document_tree::node node = doc.get_document_root(0);

    assert(node.type() == yaml_node_t::map);
    assert(node.child_count() == 3);

    yaml_document_tree::node key = node.key(0);
    assert(key.type() == yaml_node_t::string);
    assert(key.string_value() == "positive");

    key = node.key(1);
    assert(key.type() == yaml_node_t::string);
    assert(key.string_value() == "negative");

    key = node.key(2);
    assert(key.type() == yaml_node_t::string);
    assert(key.string_value() == "non boolean");

    // list of boolean true's.
    node = node.child(0);
    assert(node.type() == yaml_node_t::sequence);
    assert(node.child_count() == 11);
    for (size_t i = 0; i < node.child_count(); ++i)
    {
        yaml_document_tree::node child = node.child(i);
        assert(child.type() == yaml_node_t::boolean_true);
    }
    node = node.parent();

    // list of boolean false's.
    node = node.child(1);
    assert(node.type() == yaml_node_t::sequence);
    assert(node.child_count() == 11);
    for (size_t i = 0; i < node.child_count(); ++i)
    {
        yaml_document_tree::node child = node.child(i);
        assert(child.type() == yaml_node_t::boolean_false);
    }
    node = node.parent();

    // list of strings.
    const char* values[] = {
        "yES",
        "nO",
        "tRUE",
        "faLSE",
        "oN",
        "oFF"
    };

    node = node.child(2);
    assert(node.type() == yaml_node_t::sequence);
    assert(node.child_count() == ORCUS_N_ELEMENTS(values));

    for (size_t i = 0; i < ORCUS_N_ELEMENTS(values); ++i)
    {
        node = node.child(i);
        assert(node.type() == yaml_node_t::string);
        assert(node.string_value() == values[i]);
        node = node.parent();
    }
}

void test_yaml_parse_quoted_string()
{
    yaml_document_tree doc = load_doc(SRCDIR"/test/yaml/quoted-string/input.yaml");

    assert(doc.get_document_count() == 1);
    yaml_document_tree::node node = doc.get_document_root(0);

    assert(node.type() == yaml_node_t::map);
    assert(node.child_count() == 3);

    assert(string_expected(node.key(0), "I am quoted: ~ "));
    assert(string_expected(node.key(1), "list with quoted string values"));
    assert(string_expected(node.key(2), "single quoted string values"));
    assert(string_expected(node.child(0), "Here is another quote."));

    node = node.child(1);

    {
        // list of strings.
        const char* values[] = {
            "1 2 3",
            "null",
            "true",
            "false",
            "#hashtag",
            "\"quoted inside\"",
            "'single quote inside'",
            "Japan's finest beer"
        };

        size_t n = ORCUS_N_ELEMENTS(values);
        assert(node.type() == yaml_node_t::sequence);
        assert(node.child_count() == n);

        for (size_t i = 0; i < n; ++i)
            assert(string_expected(node.child(i), values[i]));
    }

    node = node.parent().child(2);

    {
        // list of strings.
        const char* values[] = {
            "8.8.8.8",
            "'single quote inside'",
            "prefix 'quoted' suffix",
            "\"double quote\"",
            "before \"quote\" after",
            "http://www.google.com",
            "'''",
            " ' ' ' ",
            "#hashtag"
        };

        size_t n = ORCUS_N_ELEMENTS(values);
        assert(node.type() == yaml_node_t::sequence);
        assert(node.child_count() == n);

        for (size_t i = 0; i < n; ++i)
            assert(string_expected(node.child(i), values[i]));
    }
}

void test_yaml_parse_multi_line_1()
{
    yaml_document_tree doc = load_doc(SRCDIR"/test/yaml/multi-line-1/input.yaml");

    assert(doc.get_document_count() == 1);
    yaml_document_tree::node node = doc.get_document_root(0);
    assert(string_expected(node, "1 2 3"));
    assert(node.child_count() == 0);
}

void test_yaml_parse_multi_line_2()
{
    yaml_document_tree doc = load_doc(SRCDIR"/test/yaml/multi-line-2/input.yaml");

    assert(doc.get_document_count() == 1);
    yaml_document_tree::node node = doc.get_document_root(0);
    assert(string_expected(node, "1 - 2 - 3"));
    assert(node.child_count() == 0);
}

void test_yaml_parse_literal_block_1()
{
    yaml_document_tree doc = load_doc(SRCDIR"/test/yaml/literal-block-1/input.yaml");

    assert(doc.get_document_count() == 1);
    yaml_document_tree::node node = doc.get_document_root(0);

    assert(string_expected(node, "line 1\n  line 2\nline 3\n2 blanks follow  "));
    assert(node.child_count() == 0);
}

void test_yaml_parse_literal_block_2()
{
    yaml_document_tree doc = load_doc(SRCDIR"/test/yaml/literal-block-2/input.yaml");

    assert(doc.get_document_count() == 1);
    yaml_document_tree::node node = doc.get_document_root(0);

    assert(node.type() == yaml_node_t::map);
    assert(node.child_count() == 2);

    assert(string_expected(node.key(0),   "literal block"));
    assert(string_expected(node.child(0), "line 1\n line 2\n  line 3"));
    assert(string_expected(node.key(1),   "multi line"));
    assert(string_expected(node.child(1), "line 1 line 2 line 3"));
}

void test_yaml_parse_url()
{
    yaml_document_tree doc = load_doc(SRCDIR"/test/yaml/url/input.yaml");

    assert(doc.get_document_count() == 1);
    yaml_document_tree::node node = doc.get_document_root(0);

    assert(node.type() == yaml_node_t::sequence);
    assert(node.child_count() == 3);

    assert(string_expected(node.child(0), "http://www.google.com/"));
    assert(string_expected(node.child(1), "mailto:joe@joe-me.com"));

    node = node.child(2);
    assert(node.type() == yaml_node_t::map);
    assert(node.child_count() == 2);

    assert(string_expected(node.key(0), "orcus-url"));
    assert(string_expected(node.key(1), "debian-bugs"));
    assert(string_expected(node.child(0), "http://gitlab.com/orcus/orcus"));
    assert(string_expected(node.child(1), "mailto:submit@bugs.debian.org"));
}

void test_yaml_parse_empty_value_map_1()
{
    yaml_document_tree doc = load_doc(SRCDIR"/test/yaml/empty-value-map-1/input.yaml");

    assert(doc.get_document_count() == 1);
    yaml_document_tree::node node = doc.get_document_root(0);

    assert(node.type() == yaml_node_t::map);
    assert(node.child_count() == 1);

    assert(string_expected(node.key(0), "key"));
    assert(node.child(0).type() == yaml_node_t::null);
}

void test_yaml_parse_empty_value_map_2()
{
    yaml_document_tree doc = load_doc(SRCDIR"/test/yaml/empty-value-map-2/input.yaml");

    assert(doc.get_document_count() == 1);
    yaml_document_tree::node node = doc.get_document_root(0);

    assert(node.type() == yaml_node_t::map);
    assert(node.child_count() == 2);

    assert(string_expected(node.key(0), "key1"));
    assert(node.child(0).type() == yaml_node_t::null);
    assert(string_expected(node.key(1), "key2"));
    assert(node.child(1).type() == yaml_node_t::null);
}

void test_yaml_parse_empty_value_sequence_1()
{
    yaml_document_tree doc = load_doc(SRCDIR"/test/yaml/empty-value-sequence-1/input.yaml");

    assert(doc.get_document_count() == 1);
    yaml_document_tree::node node = doc.get_document_root(0);

    assert(node.type() == yaml_node_t::sequence);
    assert(node.child_count() == 1);

    assert(node.child(0).type() == yaml_node_t::null);
}

void test_yaml_parse_empty_value_sequence_2()
{
    yaml_document_tree doc = load_doc(SRCDIR"/test/yaml/empty-value-sequence-2/input.yaml");

    assert(doc.get_document_count() == 1);
    yaml_document_tree::node node = doc.get_document_root(0);

    assert(node.type() == yaml_node_t::sequence);
    assert(node.child_count() == 2);

    assert(node.child(0).type() == yaml_node_t::null);
    assert(node.child(1).type() == yaml_node_t::null);
}

void test_yaml_map_key_1()
{
    yaml_document_tree doc = load_doc(SRCDIR"/test/yaml/map-key-1/input.yaml");

    assert(doc.get_document_count() == 1);
    yaml_document_tree::node node = doc.get_document_root(0);

    assert(node.type() == yaml_node_t::map);
    assert(node.child_count() == 2);

    assert(string_expected(node.key(0), "-key"));
    assert(string_expected(node.child(0), "value"));
    assert(string_expected(node.key(1), "key"));
    assert(string_expected(node.child(1), "value"));
}

int main()
{
    test_yaml_invalids();
    test_yaml_parse_basic1();
    test_yaml_parse_basic2();
    test_yaml_parse_basic3();
    test_yaml_parse_null();
    test_yaml_parse_boolean();
    test_yaml_parse_quoted_string();
    test_yaml_parse_multi_line_1();
    test_yaml_parse_multi_line_2();
    test_yaml_parse_literal_block_1();
    test_yaml_parse_literal_block_2();
    test_yaml_parse_url();
    test_yaml_parse_empty_value_map_1();
    test_yaml_parse_empty_value_map_2();
    test_yaml_parse_empty_value_sequence_1();
    test_yaml_parse_empty_value_sequence_2();
    test_yaml_map_key_1();

    return EXIT_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

