/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "test_global.hpp"
#include <algorithm>
#include <cstdlib>
#include <vector>

#include <orcus/zip_archive_stream.hpp>
#include <orcus/zip_archive.hpp>

#include <boost/filesystem.hpp>

#define ASSERT_THROW(expr) \
try \
{ \
    expr; \
    assert(0); \
} \
catch (...) \
{ \
}

using namespace orcus;
namespace fs = boost::filesystem;

void test_zip_archive_stream(zip_archive_stream* const strm, const unsigned char* const data, std::size_t const length)
{
    assert(strm->size() == length);
    assert(strm->tell() == 0);

    std::vector<unsigned char> buffer(length, 0);
    unsigned char* buf = buffer.data();

    strm->read(buf, 2);
    assert(std::equal(data, data + 2, buf));
    assert(strm->tell() == 0);
    strm->read(buf, length);
    assert(std::equal(data, data + length, buf));
    ASSERT_THROW(strm->read(buf, length + 1));
    strm->read(buf, 0);

    strm->seek(2);
    assert(strm->tell() == 2);
    strm->read(buf, 2);
    assert(std::equal(data + 2, data + 4, buf));
    strm->seek(length);
    assert(strm->tell() == length);
    ASSERT_THROW(strm->seek(length + 1));
    assert(strm->tell() == length);
}

void test_zip_archive_stream_blob()
{
    ORCUS_TEST_FUNC_SCOPE;

    const unsigned char data[] = "My hovercraft is full of eels.";
    zip_archive_stream_blob strm(data, sizeof(data));
    test_zip_archive_stream(&strm, data, sizeof(data));
}

void test_zip_archive_file_entry_header()
{
    ORCUS_TEST_FUNC_SCOPE;

    fs::path filepath{SRCDIR"/test/ods/raw-values-1/input.ods"};
    assert(fs::is_regular_file(filepath));

    zip_archive_stream_fd strm(filepath.string().c_str());

    zip_archive archive(&strm);
    archive.load();
    std::size_t n_entries = archive.get_file_entry_count();
    for (std::size_t i = 0; i < n_entries; ++i)
    {
        std::string_view name = archive.get_file_entry_name(i);
        std::cout << "* entry name: " << name << std::endl;
        zip_file_entry_header header = archive.get_file_entry_header(i);
        assert(header.filename == name);
        assert(header.header_signature == 0x04034b50);

        // 0 = none; 8 = deflate
        assert(header.compression_method == 0 || header.compression_method == 8);
    }
}

int main()
{
    test_zip_archive_stream_blob();
    test_zip_archive_file_entry_header();

    return EXIT_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
