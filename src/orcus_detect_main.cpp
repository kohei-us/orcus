/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <orcus/format_detection.hpp>
#include <orcus/exception.hpp>
#include <orcus/stream.hpp>

#include "cli_global.hpp"

#include <iostream>

using namespace orcus;

int ORCUS_CLI_MAIN(int argc, arg_char_t** argv) try
{
    bootstrap_program();

    if (argc != 2)
        return EXIT_FAILURE;

    auto content = to_file_content(argv[1]);

    if (content.empty())
    {
        std::cerr << "file is empty" << std::endl;
        return EXIT_FAILURE;
    }

    format_t detected_type = detect(content.str());

    std::cout << "type: ";
    switch (detected_type)
    {
        case format_t::csv:
            std::cout << "plain text format";
            break;
        case format_t::gnumeric:
            std::cout << "Gnumeric";
            break;
        case format_t::ods:
            std::cout << "OpenDocument Spreadsheet";
            break;
        case format_t::xls_xml:
            std::cout << "Microsoft Excel XML";
            break;
        case format_t::xlsx:
            std::cout << "Microsoft Office Open XML Excel 2007+";
            break;
        case format_t::parquet:
            std::cout << "Apache Parquet";
            break;
        case format_t::json:
            std::cout << "JSON text data";
            break;
        case format_t::xml:
            std::cout << "XML document";
            break;
        case format_t::unknown:
        default:
            std::cout << "unknown";
    }
    std::cout << std::endl;

    return EXIT_SUCCESS;
}
catch (const std::exception& e)
{
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
