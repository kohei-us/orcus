/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "test_global.hpp"

#include <orcus/stream.hpp>
#include <orcus/orcus_parquet.hpp>
#include <orcus/spreadsheet/document.hpp>
#include <orcus/spreadsheet/factory.hpp>

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <iostream>
#include <sstream>

using namespace orcus;
namespace fs = boost::filesystem;

void test_parquet_basic()
{
    const fs::path test_doc_dir{SRCDIR"/test/parquet/basic"};

    constexpr std::string_view test_docs[] = {
        "basic-gzip.parquet",
        "basic-nocomp.parquet",
        "basic-snappy.parquet",
        "basic-zstd.parquet",
    };

    for (auto test_doc : test_docs)
    {
        const auto docpath = test_doc_dir / std::string{test_doc};
        assert(fs::is_regular_file(docpath));

        spreadsheet::range_size_t ss{1048576, 16384};
        spreadsheet::document doc{ss};
        spreadsheet::import_factory fact(doc);
        orcus_parquet app(&fact);

        app.read_file(docpath.native());
        assert(doc.get_sheet_count() == 1);

        // Dump the content of the model.
        std::ostringstream os;
        doc.dump_check(os);
        std::string check = os.str();

        const fs::path check_path = test_doc_dir / (docpath.filename().string() + ".check");
        file_content control{check_path.native()};

        test::verify_content(__FILE__, __LINE__, control.str(), check);
    }
}

int main()
{
    try
    {
        test_parquet_basic();
    }
    catch (const std::exception& e)
    {
        std::cout << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

