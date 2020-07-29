/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "test_global.hpp"
#include "orcus/xml_writer.hpp"
#include "orcus/xml_namespace.hpp"
#include "orcus/sax_parser.hpp"

#include <iostream>
#include <sstream>

using namespace orcus;

void test_encoded_content()
{
    const std::vector<std::string> test_contents = {
        "1 < 2 but 3 > 2",
        "ladies & gentlemen",
        "'testing single quotes'",
        "\"testing double quotes\"",
    };

    struct _handler : public sax_handler
    {
        std::ostringstream os_content;

        void characters(const orcus::pstring& val, bool transient)
        {
            os_content << val;
        }
    };

    for (const std::string& test_content : test_contents)
    {
        xmlns_repository repo;
        std::ostringstream os;

        {
            xml_writer writer(repo, os);
            auto scope_root = writer.set_element_scope({nullptr, "root"});
            writer.add_content(test_content);
        }

        std::string stream = os.str();

        _handler hdl;

        sax_parser<_handler> parser(stream.data(), stream.size(), hdl);
        parser.parse();

        std::string content_read = hdl.os_content.str();
        assert(test_content == content_read);
    }
}

void test_move()
{
    xmlns_repository repo;

    {
        std::ostringstream os;
        xml_writer writer(repo, os);

        writer.push_element({nullptr, "foo"});

        {
            xml_writer moved(std::move(writer)); // move constructor
            moved.add_content("stuff");
        }

        std::string stream = os.str();
        assert(stream == "<?xml version=\"1.0\"?><foo>stuff</foo>");
    }

    {
        std::ostringstream os;
        xml_writer writer(repo, os);

        writer.push_element({nullptr, "foo2"});

        {
            std::ostringstream os2;
            xml_writer moved(repo, os2);

            moved = std::move(writer); // move assignment.
            moved.add_content("stuff2");
        }

        std::string stream = os.str();
        assert(stream == "<?xml version=\"1.0\"?><foo2>stuff2</foo2>");
    }
}

int main()
{
    test_encoded_content();
    test_move();

    return EXIT_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
