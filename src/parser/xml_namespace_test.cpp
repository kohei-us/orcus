/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "test_global.hpp"
#include "orcus/xml_namespace.hpp"
#include "pstring.hpp"

#include <cstdlib>
#include <vector>
#include <algorithm>

using namespace std;
using namespace orcus;

namespace {

void test_basic()
{
    ORCUS_TEST_FUNC_SCOPE;

    pstring xmlns1("http://some.xmlns/");
    pstring xmlns2("http://other.xmlns/");

    xmlns_repository repo;
    xmlns_context cxt1 = repo.create_context();
    xmlns_context cxt2 = repo.create_context();

    pstring empty, myns("myns");
    {
        // context 1
        xmlns_id_t test1 = cxt1.push(empty, xmlns1); // register default namespace.
        assert(cxt1.get(empty) == test1);
        xmlns_id_t test2 = cxt1.push(myns, xmlns2);
        assert(cxt1.get(myns) == test2);
        assert(test1 != test2);
    }

    {
        // context 2
        xmlns_id_t test1 = cxt2.push(empty, xmlns2); // register default namespace.
        assert(cxt2.get(empty) == test1);
        xmlns_id_t test2 = cxt2.push(myns, xmlns1);
        assert(cxt2.get(myns) == test2);
        assert(test1 != test2);
    }

    // Now, compare the registered namespaces between the two namespaces.
    assert(cxt1.get(empty) == cxt2.get(myns));
    assert(cxt1.get(myns) == cxt2.get(empty));
}

void test_all_namespaces()
{
    ORCUS_TEST_FUNC_SCOPE;

    pstring key1("a"), key2("b"), key3("c");
    pstring ns1("foo"), ns2("baa"), ns3("hmm");

    xmlns_repository repo;
    xmlns_context cxt = repo.create_context();
    xmlns_id_t ns;

    ns = cxt.push(key1, ns1);
    assert(ns1 == ns);
    ns = cxt.push(key2, ns2);
    assert(ns2 == ns);
    ns = cxt.push(key3, ns3);
    assert(ns3 == ns);

    vector<xmlns_id_t> all_ns = cxt.get_all_namespaces();
    assert(all_ns.size() == 3);
    assert(ns1 == all_ns[0]);
    assert(ns2 == all_ns[1]);
    assert(ns3 == all_ns[2]);
}

const xmlns_id_t NS_test_name1 = "test:name:1";
const xmlns_id_t NS_test_name2 = "test:name:2";
const xmlns_id_t NS_test_name3 = "test:name:3";

xmlns_id_t NS_test_all[] = {
    NS_test_name1,
    NS_test_name2,
    NS_test_name3,
    nullptr
};

xmlns_id_t NS_test_all_reverse[] = {
    NS_test_name3,
    NS_test_name2,
    NS_test_name1,
    nullptr
};

void test_predefined_ns()
{
    xmlns_repository ns_repo;
    ns_repo.add_predefined_values(NS_test_all);
    xmlns_context cxt = ns_repo.create_context();
    xmlns_id_t ns_id = cxt.push("tn1", "test:name:1");
    assert(ns_id == NS_test_name1);
    ns_id = cxt.push("tn2", "test:name:2");
    assert(ns_id == NS_test_name2);
    ns_id = cxt.push("tn3", "test:name:3");
    assert(ns_id == NS_test_name3);
    assert(cxt.get("tn1") == NS_test_name1);
    assert(cxt.get("tn2") == NS_test_name2);
    assert(cxt.get("tn3") == NS_test_name3);
}

void test_xml_name_t()
{
    ORCUS_TEST_FUNC_SCOPE;

    xml_name_t name1;
    name1.ns = NS_test_name1;
    name1.name = "foo";

    xml_name_t name2 = name1;
    assert(name1 == name2);

    name2.name = "foo2";
    assert(name1 != name2);

    xml_name_t name3 = name1;
    name3.ns = NS_test_name2;
    assert(name1 != name3);
}

void test_ns_context()
{
    ORCUS_TEST_FUNC_SCOPE;

    xmlns_repository repo;
    repo.add_predefined_values(NS_test_all);

    xmlns_repository repo2;
    repo2.add_predefined_values(NS_test_all_reverse);

    xmlns_context cxt;
    cxt = repo.create_context(); // copy assignment
    size_t id1 = cxt.get_index(NS_test_name3);
    xmlns_context cxt2 = cxt; // copy ctor
    size_t id2 = cxt2.get_index(NS_test_name3);

    assert(id1 == id2);

    xmlns_context cxt3 = repo2.create_context();
    id2 = cxt3.get_index(NS_test_name3);

    assert(id1 != id2);

    cxt3 = std::move(cxt2); // move assignment
    id2 = cxt3.get_index(NS_test_name3);

    assert(id1 == id2);

    try
    {
        id1 = cxt2.get_index(NS_test_name2);
        assert(!"exception was supposed to be thrown due to no associated repos.");
    }
    catch (const std::exception&)
    {
        // expected
    }

    xmlns_context cxt4(std::move(cxt3)); // move ctor
    id1 = cxt4.get_index(NS_test_name3);

    xmlns_context cxt5 = repo.create_context();
    id2 = cxt5.get_index(NS_test_name3);

    assert(id1 == id2);

    try
    {
        id1 = cxt3.get_index(NS_test_name2);
        assert(!"exception was supposed to be thrown due to no associated repos.");
    }
    catch (const std::exception&)
    {
        // expected
    }

    cxt4 = repo.create_context();
    cxt5 = repo2.create_context();
    id1 = cxt4.get_index(NS_test_name1);
    id2 = cxt5.get_index(NS_test_name1);

    assert(id1 != id2);

    cxt3 = repo.create_context();
    cxt5.swap(cxt3);
    id2 = cxt5.get_index(NS_test_name1);

    assert(id1 == id2);
}

void test_repo_move()
{
    ORCUS_TEST_FUNC_SCOPE;

    static_assert(!std::is_copy_constructible_v<xmlns_repository>);
    static_assert(std::is_move_constructible_v<xmlns_repository>);

    xmlns_repository repo;
    repo.add_predefined_values(NS_test_all);

    xmlns_repository repo_moved = std::move(repo); // move construction
    xmlns_repository repo_moved2;
    repo_moved2 = std::move(repo_moved); // move assignment

    xmlns_id_t ns_id = repo_moved2.get_identifier(0);
    assert(ns_id != XMLNS_UNKNOWN_ID);
    ns_id = repo_moved2.get_identifier(1);
    assert(ns_id != XMLNS_UNKNOWN_ID);
    ns_id = repo_moved2.get_identifier(2);
    assert(ns_id != XMLNS_UNKNOWN_ID);
    ns_id = repo_moved2.get_identifier(3);
    assert(ns_id == XMLNS_UNKNOWN_ID);
}

} // anonymous namespace

int main()
{
    test_basic();
    test_all_namespaces();
    test_predefined_ns();
    test_xml_name_t();
    test_ns_context();
    test_repo_move();

    return EXIT_SUCCESS;
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
