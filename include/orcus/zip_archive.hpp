/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_ORCUS_ZIP_ARCHIVE_HPP
#define INCLUDED_ORCUS_ZIP_ARCHIVE_HPP

#include "env.hpp"
#include "exception.hpp"

#include <string_view>
#include <vector>
#include <memory>
#include <ostream>

namespace orcus {

/**
 * Structure containing file entry header attributes.
 */
struct ORCUS_PSR_DLLPUBLIC zip_file_entry_header
{
    uint32_t header_signature = 0;
    uint16_t required_version = 0;
    uint16_t flag = 0;
    uint16_t compression_method = 0;
    uint16_t last_modified_time = 0;
    uint16_t last_modified_date = 0;
    uint32_t crc32 = 0;
    uint32_t compressed_size = 0;
    uint32_t uncompressed_size = 0;

    std::string filename;
    std::vector<uint8_t> extra_field;

    zip_file_entry_header();
    zip_file_entry_header(const zip_file_entry_header& other);
    zip_file_entry_header(zip_file_entry_header&& other);
    ~zip_file_entry_header();

    zip_file_entry_header& operator=(const zip_file_entry_header& other);
    zip_file_entry_header& operator=(zip_file_entry_header&& other);
};

ORCUS_PSR_DLLPUBLIC std::ostream& operator<<(std::ostream& os, const zip_file_entry_header& header);

class zip_archive_stream;

class ORCUS_PSR_DLLPUBLIC zip_archive
{
    struct impl;

    std::unique_ptr<impl> mp_impl;

public:
    zip_archive() = delete;
    zip_archive(const zip_archive&) = delete;
    zip_archive& operator= (const zip_archive) = delete;

    zip_archive(zip_archive_stream* stream);
    ~zip_archive();

    /**
     * Loading involves the parsing of the central directory of a zip archive
     * (located toward the end of the stream) and building of file entry data
     * which are stored in the central directory.
     */
    void load();

    /**
     * Retrieve the header information for a file entry specified by index.
     *
     * @param index file entry index.
     *
     * @return header information for a file entry.
     */
    zip_file_entry_header get_file_entry_header(std::size_t index) const;

    /**
     * Retrieve the header information for a file entry specified by name.
     *
     * @param name file entry name.
     *
     * @return header information for a file entry.
     */
    zip_file_entry_header get_file_entry_header(std::string_view name) const;

    /**
     * Get file entry name from its index.
     *
     * @param index file entry index
     *
     * @return file entry name
     */
    std::string_view get_file_entry_name(std::size_t index) const;

    /**
     * Return the number of file entries stored in this zip archive.  Note
     * that a file entry may be a directory, so the number of files stored in
     * the zip archive may not equal the number of file entries.
     *
     * @return number of file entries.
     */
    size_t get_file_entry_count() const;

    /**
     * Retrieve data stream of specified file entry. The retrieved data stream
     * gets uncompressed if the original stream is compressed.
     *
     * @param entry_name file entry name.
     * @param buf buffer to put the retrieved data stream into.
     *
     * @return buffer containing the data stream for specified entry.
     *
     * @exception zip_error thrown when any problem is encountered during data
     *                      stream retrieval.
     */
    std::vector<unsigned char> read_file_entry(std::string_view entry_name) const;
};

}

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
