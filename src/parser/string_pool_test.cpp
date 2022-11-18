/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "test_global.hpp"
#include <orcus/string_pool.hpp>

#include <type_traits>

using namespace orcus;

void test_basic()
{
    const char* static_text = "test";

    string_pool pool;
    assert(pool.size() == 0);

    std::pair<std::string_view, bool> ret = pool.intern("foo");
    assert(ret.first == "foo");
    assert(ret.second); // new instance

    ret = pool.intern("foo");
    assert(ret.first == "foo");
    assert(!ret.second); // existing instance.

    // Empty strings should not be interned.
    ret = pool.intern("");
    assert(ret.first.empty());
    assert(!ret.second);

    ret = pool.intern("A");
    std::cout << "interned string: " << ret.first << std::endl;
    assert(ret.second);
    assert(pool.size() == 2);

    // Duplicate string.
    ret = pool.intern("A");
    std::cout << "interned string: " << ret.first << std::endl;
    assert(!ret.second);

    ret = pool.intern("B");
    std::cout << "interned string: " << ret.first << std::endl;
    assert(pool.size() == 3);

    // Interning an already-intern string should return a pstring with
    // identical memory address.
    std::string_view str = ret.first;
    std::string_view str2 = pool.intern(str).first;
    assert(str == str2);
    assert(pool.size() == 3);
    assert(str.data() == str2.data()); // their memory address should be identical.

    std::string_view static_str(static_text);
    ret = pool.intern(static_str);
    str = ret.first;
    std::cout << "interned string: " << str << std::endl;
    assert(pool.size() == 4);
    assert(str == static_str);
    assert(str.data() != static_str.data());

    // Make sure that the pool remains usable after calling clear().
    pool.clear();
    assert(pool.size() == 0);
    ret = pool.intern(static_str);
    assert(ret.second); // it should be a new string
    assert(ret.first == static_str);
}

void test_merge()
{
    string_pool pool1;
    std::unique_ptr<string_pool> pool2(std::make_unique<string_pool>());

    pool1.intern("A");
    pool1.intern("B");
    pool1.intern("C");
    std::string_view v1 = pool1.intern("same value").first;

    pool2->intern("D");
    pool2->intern("E");
    pool2->intern("F");
    std::string_view v2 = pool2->intern("same value").first;

    assert(pool1.size() == 4);
    assert(pool2->size() == 4);

    pool1.merge(*pool2);

    assert(pool1.size() == 7);
    assert(pool2->size() == 0);

    pool2.reset(); // Delete the pool2 instance altogether.

    // This should not create a new entry.
    auto r = pool1.intern("F");
    assert(!r.second);

    // v2 still points to the original string in pool2, which should now be in
    // the merged store in pool1 (thus valid).
    assert(v1 == v2);

    std::vector<std::string_view> entries = pool1.get_interned_strings();
    assert(entries.size() == pool1.size());
}

void test_move()
{
    static_assert(!std::is_copy_constructible_v<orcus::string_pool>);
    static_assert(std::is_move_constructible_v<orcus::string_pool>);

    string_pool pool1;
    pool1.intern("A");
    pool1.intern("B");
    pool1.intern("C");
    pool1.intern("D");
    pool1.intern("E");

    string_pool pool2 = std::move(pool1);
    assert(pool2.size() == 5);
}

int main()
{
    test_basic();
    test_merge();
    test_move();

    return EXIT_SUCCESS;
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
