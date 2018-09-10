/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "orcus/css_document_tree.hpp"
#include "orcus/stream.hpp"

#include <iostream>

int main(int argc, char** argv)
{
    if (argc < 2)
        return EXIT_FAILURE;

    const char* filepath = argv[1];
    try
    {
        std::string strm = orcus::load_file_content(filepath);
        orcus::css_document_tree doc;
        doc.load(strm);
        doc.dump();
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
