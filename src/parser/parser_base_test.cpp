/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "test_global.hpp"
#include "orcus/parser_base.hpp"
#include "orcus/global.hpp"

using namespace std;
using namespace orcus;

void test_skip_space_and_control()
{
    class _test_type : public orcus::parser_base
    {
    public:
        _test_type(const char* p, size_t n) : orcus::parser_base(p, n, false) {}

        void run()
        {
            skip_space_and_control();
        }

        bool has_char() const
        {
            return orcus::parser_base::has_char();
        }

        size_t available_size() const
        {
            return orcus::parser_base::available_size();
        }

        char get_char() const
        {
            return *mp_char;
        }
    };

    // Create a series of variable-legnth blank strings and make sure the
    // function correctly skips all the empty characters.

    for (size_t i = 0; i < 32; ++i)
    {
        std::string s(i, ' ');
        assert(s.size() == i);

        _test_type test(s.data(), s.size());
        assert(test.available_size() == s.size());

        test.run();
        assert(!test.has_char()); // There should be no more characters to parse.

        s.push_back('a');

        _test_type test2(s.data(), s.size());
        assert(test2.available_size() == s.size());

        test2.run();
        assert(test2.has_char()); // The current position should be on the 'a'.
        assert(test2.get_char() == 'a');
    }
}

int main()
{
    test_skip_space_and_control();

    return EXIT_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
