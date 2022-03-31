/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <orcus/format_detection.hpp>
#include <orcus/stream.hpp>

#include <cassert>
#include <iostream>
#include <string>

#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

fs::path base_test_dir = fs::path{SRCDIR} / "test";

void test_detect_formats()
{
    struct {
        fs::path path;
        orcus::format_t format;
    } tests[] = {
        { base_test_dir / "ods" / "raw-values-1" / "input.ods", orcus::format_t::ods },
        { base_test_dir / "xlsx" / "raw-values-1" / "input.xlsx", orcus::format_t::xlsx },
        { base_test_dir / "xls-xml" / "basic" / "input.xml", orcus::format_t::xls_xml },
        { base_test_dir / "gnumeric" / "test.gnumeric", orcus::format_t::gnumeric }
    };

    for (size_t i = 0; i < std::size(tests); ++i)
    {
        orcus::file_content content(tests[i].path.string());
        assert(!content.empty());
        orcus::format_t detected = orcus::detect(
            reinterpret_cast<const unsigned char*>(content.data()), content.size());

        assert(detected == tests[i].format);
    }
}

int main()
{
    test_detect_formats();

    return EXIT_SUCCESS;
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
