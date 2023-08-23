/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "test_global.hpp"
#include <orcus/sax_parser.hpp>
#include <orcus/stream.hpp>

#include <iostream>
#include <boost/range/iterator_range.hpp>

#include "filesystem_env.hpp"

void test_valid()
{
    ORCUS_TEST_FUNC_SCOPE;

    struct _handler : public orcus::sax_handler {};

    fs::path root_dir = fs::path{SRCDIR} / "test" / "xml" / "valids";

    if (!fs::is_directory(root_dir))
        return;

    for (const fs::path& entry : boost::make_iterator_range(fs::directory_iterator{root_dir}, {}))
    {
        std::cout << "input file: " << entry << std::endl;

        orcus::file_content content(entry.string());

        _handler hdl;
        orcus::sax_parser<_handler> parser(content.str(), hdl);

        try
        {
            parser.parse();
        }
        catch (const orcus::malformed_xml_error& e)
        {
            std::cerr << orcus::create_parse_error_output(content.str(), e.offset()) << std::endl;
            std::cerr << e.what() << std::endl;
            assert(!"This was supposed to be a valid XML!");

        }
    }
}

void test_invalid()
{
    ORCUS_TEST_FUNC_SCOPE;

    struct _handler : public orcus::sax_handler {};

    fs::path root_dir = fs::path{SRCDIR} / "test" / "xml" / "invalids";

    if (!fs::is_directory(root_dir))
        return;

    for (const fs::path& entry : boost::make_iterator_range(fs::directory_iterator{root_dir}, {}))
    {
        std::cout << "input file: " << entry << std::endl;

        orcus::file_content content(entry.string());

        _handler hdl;
        orcus::sax_parser<_handler> parser(content.str(), hdl);

        try
        {
            parser.parse();
            assert(!"exception was expected, but one was not thrown.");
        }
        catch (const orcus::malformed_xml_error& e)
        {
            std::cerr << orcus::create_parse_error_output(content.str(), e.offset()) << std::endl;
            std::cerr << e.what() << std::endl;
        }
        catch (...)
        {
            assert(!"wrong exception was thrown.");
        }
    }
}

int main()
{
    test_valid();
    test_invalid();

    return EXIT_SUCCESS;
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
