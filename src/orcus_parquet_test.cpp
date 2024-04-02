/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "orcus_test_global.hpp"

#include <orcus/stream.hpp>
#include <orcus/orcus_parquet.hpp>
#include <orcus/format_detection.hpp>
#include <orcus/spreadsheet/document.hpp>
#include <orcus/spreadsheet/factory.hpp>

#include <iostream>
#include <sstream>

#include "filesystem_env.hpp"

using namespace orcus;
namespace ss = orcus::spreadsheet;

const fs::path BASIC_TEST_DOC_DIR = SRCDIR"/test/parquet/basic";

constexpr std::string_view BASIC_TEST_DOCS[] = {
    "basic-gzip.parquet",
    "basic-nocomp.parquet",
    "basic-snappy.parquet",
    "basic-zstd.parquet",
    "float-with-nan.parquet",
};

struct doc_context
{
    ss::document doc;
    ss::import_factory factory;
    orcus_parquet app;

    doc_context() :
        doc{ss::range_size_t{1048576, 16384}}, factory{doc}, app{&factory}
    {
    }

    std::string get_check_string() const
    {
        std::ostringstream os;
        doc.dump_check(os);
        return os.str();
    }
};

void test_parquet_create_filter()
{
    ORCUS_TEST_FUNC_SCOPE;

    ss::range_size_t ssize{1048576, 16384};
    std::unique_ptr<ss::document> doc = std::make_unique<ss::document>(ssize);
    ss::import_factory factory(*doc);

    auto f = create_filter(format_t::parquet, &factory);
    assert(f);
    assert(f->get_name() == "parquet");
}

void test_parquet_basic()
{
    ORCUS_TEST_FUNC_SCOPE;

    for (auto test_doc : BASIC_TEST_DOCS)
    {
        const auto docpath = BASIC_TEST_DOC_DIR / std::string{test_doc};
        std::cout << docpath << std::endl;
        assert(fs::is_regular_file(docpath));

        // Test the file import.
        auto cxt = std::make_unique<doc_context>();
        cxt->app.read_file(docpath.string());
        assert(cxt->doc.get_sheet_count() == 1);

        // Check the content vs control
        const fs::path check_path = BASIC_TEST_DOC_DIR / (docpath.filename().string() + ".check");
        file_content control{check_path.string()};

        test::verify_content(__FILE__, __LINE__, control.str(), cxt->get_check_string());

        // Test the stream import.  Manually change the sheet name to the stem
        // of the input file since the sheet name is set to 'Data' for stream
        // imports.
        cxt = std::make_unique<doc_context>();
        file_content fc(docpath.string());
        cxt->app.read_stream(fc.str());
        cxt->doc.set_sheet_name(0, docpath.stem().string());

        // Check the content vs control
        test::verify_content(__FILE__, __LINE__, control.str(), cxt->get_check_string());
    }
}

void test_parquet_detection()
{
    ORCUS_TEST_FUNC_SCOPE;

    for (auto test_doc : BASIC_TEST_DOCS)
    {
        const auto docpath = BASIC_TEST_DOC_DIR / std::string{test_doc};
        assert(fs::is_regular_file(docpath));

        file_content content{docpath.string()};
        auto strm = content.str();
        bool res = orcus_parquet::detect(reinterpret_cast<const unsigned char*>(strm.data()), strm.size());
        assert(res);
    }
}

int main()
{
    try
    {
        test_parquet_create_filter();
        test_parquet_basic();
        test_parquet_detection();
    }
    catch (const std::exception& e)
    {
        std::cout << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

