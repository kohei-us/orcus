/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <orcus/base64.hpp>

#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/binary_from_base64.hpp>
#include <boost/archive/iterators/transform_width.hpp>

using namespace boost::archive::iterators;

namespace orcus {

typedef transform_width<binary_from_base64<std::vector<char>::const_iterator>, 8, 6> to_binary;
typedef base64_from_binary<transform_width<std::vector<uint8_t>::const_iterator, 6, 8> > to_base64;

std::vector<uint8_t> decode_from_base64(std::string_view base64)
{
    if (base64.size() < 4)
        // Minimum of 4 characters required.
        return std::vector<uint8_t>{};

    std::vector<char> base64_seq{base64.data(), base64.data() + base64.size()};

    // Check the number of trailing '='s (up to 2).
    std::size_t pad_size = 0;
    auto it = base64_seq.rbegin();
    for (; pad_size < 2; ++pad_size, ++it)
    {
        if (*it != '=')
            break;

        *it = 'A'; // replace it with 'A' which is a base64 encoding of '\0'.
    }

    std::vector<uint8_t> decoded{to_binary(base64_seq.begin()), to_binary(base64_seq.end())};
    decoded.erase(decoded.end() - pad_size, decoded.end());

    return decoded;
}

std::string encode_to_base64(const std::vector<uint8_t>& input)
{
    if (input.empty())
        return std::string{};

    std::vector<uint8_t> inp = input;
    size_t pad_size = (3 - inp.size() % 3) % 3;
    inp.resize(inp.size() + pad_size);

    std::string encoded{to_base64(inp.begin()), to_base64(inp.end())};

    auto it = encoded.rbegin();
    for (size_t i = 0; i < pad_size; ++i, ++it)
    {
        // 'A' is a base64 encoding of '\0'
        // replace them with padding charachters '='
        if (*it == 'A')
            *it = '=';
    }

    return encoded;
}

}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
