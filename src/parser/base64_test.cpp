/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "test_global.hpp"
#include "orcus/base64.hpp"

#include <cstdlib>
#include <cstring>
#include <vector>
#include <string>
#include <iterator>

using namespace orcus;
using namespace std;

void test_base64_text_input(const char* p)
{
    cout << "input: '" << p << "'" << endl;
    size_t n = strlen(p);
    std::vector<uint8_t> input(p, p+n);
    std::string encoded = encode_to_base64(input);
    cout << "encoded: '" << encoded << "'" << endl;

    std::vector<uint8_t> decoded = decode_from_base64(encoded);
    cout << "decoded: '";
    std::copy(decoded.begin(), decoded.end(), std::ostream_iterator<char>(cout, ""));
    cout << "'" << endl;

    assert(input == decoded);
}

int main()
{
    test_base64_text_input("Hello there");
    test_base64_text_input("World domination!!!");
    test_base64_text_input("World domination!!");
    test_base64_text_input("World domination!");
    return EXIT_SUCCESS;
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
