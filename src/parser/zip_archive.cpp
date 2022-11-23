/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <orcus/zip_archive.hpp>
#include <orcus/zip_archive_stream.hpp>
#include <orcus/string_pool.hpp>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <unordered_map>
#include <sstream>
#include <string_view>
#include <iomanip>

#include <zlib.h>
#include <zconf.h>

#define ORCUS_DEBUG_ZIP_ARCHIVE 0

namespace orcus {

namespace {

struct zip_file_param
{
    enum compress_method_type { stored = 0, deflated = 8 };

    std::string_view filename;
    compress_method_type compress_method;
    std::size_t offset_file_header;
    std::size_t size_compressed;
    std::size_t size_uncompressed;

    uint16_t version_made_by;
    uint16_t minimum_version_needed;
    uint16_t flags;
    uint16_t last_modified_time;
    uint16_t last_modified_date;

    uint16_t filename_length;
    uint16_t extra_field_length;
    uint16_t comment_length;

    uint16_t disk_id_where_file_starts;
    uint16_t file_attributes_internal;
    uint32_t file_attributes_external;

    uint32_t crc32;
};

class zip_inflater
{
    z_stream m_zlib_cxt;

    zip_inflater(); // disabled
public:
    zip_inflater(std::vector<unsigned char>& raw_buf, std::vector<unsigned char>& dest_buf, const zip_file_param& param)
    {
        memset(&m_zlib_cxt, 0, sizeof(m_zlib_cxt));
        m_zlib_cxt.next_in = static_cast<Bytef*>(&raw_buf[0]);
        m_zlib_cxt.avail_in = param.size_compressed;

        m_zlib_cxt.next_out = static_cast<Bytef*>(&dest_buf[0]);
        m_zlib_cxt.avail_out = param.size_uncompressed;
    }

    ~zip_inflater()
    {
        inflateEnd(&m_zlib_cxt);
    }

    bool init()
    {
        int err = inflateInit2(&m_zlib_cxt, -MAX_WBITS);
        return err == Z_OK;
    }

    bool inflate()
    {
        int err = ::inflate(&m_zlib_cxt, Z_SYNC_FLUSH);
        if (err >= 0 && m_zlib_cxt.msg)
            return false;

        return true;
    }
};

/**
 * Stream doesn't know its size; only its starting offset position within
 * the file stream.
 */
class zip_stream_parser
{
    zip_archive_stream* m_stream;
    size_t m_pos;
    size_t m_pos_internal;

    void read_string_to_buffer(size_t n, std::vector<unsigned char>& buf)
    {
        if (!n)
            throw zip_error("attempt to read string of zero size.");

        m_stream->seek(m_pos+m_pos_internal);
        m_stream->read(&buf[0], n);
        m_pos_internal += n;
    }

public:
    zip_stream_parser() : m_stream(nullptr), m_pos(0), m_pos_internal(0) {}
    zip_stream_parser(zip_archive_stream* stream, size_t pos) : m_stream(stream), m_pos(pos), m_pos_internal(0) {}

    std::string read_string(size_t n)
    {
        std::vector<unsigned char> buf(n+1, '\0');
        read_string_to_buffer(n, buf);
        return std::string(reinterpret_cast<const char*>(&buf[0]));
    }

    std::vector<uint8_t> read_bytes(std::size_t n)
    {
        if (!n)
            throw zip_error("attempt to read string of zero size.");

        std::vector<uint8_t> buf;
        m_stream->seek(m_pos+m_pos_internal);
        m_stream->read(buf.data(), n);
        m_pos_internal += n;
        return buf;
    }

    std::string_view read_string(size_t n, string_pool& pool)
    {
        std::vector<unsigned char> buf(n+1, '\0');
        read_string_to_buffer(n, buf);
        return pool.intern({reinterpret_cast<const char*>(buf.data()), n}).first;
    }

    void skip_bytes(size_t n)
    {
        m_pos_internal += n;
    }

    uint32_t read_4bytes()
    {
        m_stream->seek(m_pos+m_pos_internal);
        unsigned char buf[4];
        m_stream->read(&buf[0], 4);
        m_pos_internal += 4;

        uint32_t ret = buf[0];
        ret |= (buf[1] << 8);
        ret |= (buf[2] << 16);
        ret |= (buf[3] << 24);

        return ret;
    }

    uint16_t read_2bytes()
    {
        m_stream->seek(m_pos+m_pos_internal);
        unsigned char buf[2];
        m_stream->read(&buf[0], 2);
        m_pos_internal += 2;

        uint16_t ret = buf[0];
        ret |= (buf[1] << 8);

        return ret;
    }

    size_t tell() const
    {
        return m_pos + m_pos_internal;
    }
};

/**
 * Content of the end part of the central directory.
 */
struct central_dir_end
{
    uint32_t magic_number;
    uint16_t this_disk_id;
    uint16_t central_dir_disk_id;
    uint16_t num_central_dir_records_local;
    uint16_t num_celtral_dir_records_total;
    uint32_t size_central_dir;
    size_t central_dir_pos;
    uint16_t comment_length;
};

} // anonymous namespace


zip_file_entry_header::zip_file_entry_header() = default;
zip_file_entry_header::zip_file_entry_header(const zip_file_entry_header& other) = default;
zip_file_entry_header::zip_file_entry_header(zip_file_entry_header&& other) = default;
zip_file_entry_header::~zip_file_entry_header() = default;

zip_file_entry_header& zip_file_entry_header::operator=(const zip_file_entry_header& other) = default;
zip_file_entry_header& zip_file_entry_header::operator=(zip_file_entry_header&& other) = default;

std::ostream& operator<<(std::ostream& os, const zip_file_entry_header& header)
{
    os << "header signature: 0x" << std::hex << std::setfill('0') << std::setw(8) << header.header_signature << "\n"
       << "version needed to extract: " << header.required_version << "\n"
       << "general purpose bit flag: 0x" << std::hex << std::setfill('0') << std::setw(4) << header.flag << "\n"
       << "compression method: " << header.compression_method << "\n"
       << "last modified time: " << header.last_modified_time << "\n"
       << "last modified date: " << header.last_modified_date << "\n"
       << "crc32: 0x" << std::hex << std::setfill('0') << std::setw(8) << header.crc32 << "\n"
       << "compressed size: " << header.compressed_size << "\n"
       << "uncompressed size: " << header.uncompressed_size << "\n"
       << "filename: " << header.filename << "\n"
       << "extra field length: " << header.extra_field.size();

    return os;
}

class zip_archive::impl
{
    typedef std::vector<zip_file_param> file_params_type;
    typedef std::unordered_map<std::string_view, std::size_t> filename_map_type;

    string_pool m_pool;
    zip_archive_stream* m_stream;
    off_t m_stream_size;
    size_t m_central_dir_pos;

    zip_stream_parser m_central_dir_end;

    file_params_type m_file_params;
    filename_map_type m_filenames;

public:
    impl(zip_archive_stream* stream);

    void load();
    zip_file_entry_header get_file_entry_header(std::size_t index) const;
    zip_file_entry_header get_file_entry_header(std::string_view name) const;
    std::string_view get_file_entry_name(size_t pos) const;

    size_t get_file_entry_count() const
    {
        return m_file_params.size();
    }

    std::vector<unsigned char> read_file_entry(std::string_view entry_name) const;

private:

    /**
     * Find the central directory of a zip file, located toward the end before
     * the global comment, and starts with the byte sequence of 0x504b0506.
     */
    size_t seek_central_dir();

    void read_central_dir_end();
    void read_file_entries();
};

zip_archive::impl::impl(zip_archive_stream* stream) :
    m_stream(stream), m_stream_size(0), m_central_dir_pos(0)
{
    if (!m_stream)
        throw zip_error("null stream is not allowed.");

    m_stream_size = m_stream->size();
}

void zip_archive::impl::load()
{
    size_t central_dir_end_pos = seek_central_dir();
    if (!central_dir_end_pos)
        throw zip_error("failed to seek the end position of the central directory");

    m_central_dir_end = zip_stream_parser(m_stream, central_dir_end_pos);

    // Read the end part of the central directory.
    read_central_dir_end();

    // Read file entries that are in the front part of the central directory.
    read_file_entries();
}

zip_file_entry_header zip_archive::impl::get_file_entry_header(std::size_t index) const
{
    if (index >= m_file_params.size())
        throw zip_error("invalid file entry index.");

    const zip_file_param& param = m_file_params[index];
    zip_stream_parser file_header(m_stream, param.offset_file_header);

    zip_file_entry_header header;

    header.header_signature = file_header.read_4bytes();
    header.required_version = file_header.read_2bytes();
    header.flag = file_header.read_2bytes();
    header.compression_method = file_header.read_2bytes();
    header.last_modified_time = file_header.read_2bytes();
    header.last_modified_date = file_header.read_2bytes();
    header.crc32 = file_header.read_4bytes();
    header.compressed_size = file_header.read_4bytes();
    header.uncompressed_size = file_header.read_4bytes();
    uint16_t filename_len = file_header.read_2bytes();
    uint16_t extra_field_len = file_header.read_2bytes();

    if (filename_len)
        header.filename = file_header.read_string(filename_len);

    if (extra_field_len)
        header.extra_field = file_header.read_bytes(extra_field_len);

    return header;
}

zip_file_entry_header zip_archive::impl::get_file_entry_header(std::string_view name) const
{
    auto it = m_filenames.find(name);
    if (it == m_filenames.end())
    {
        std::ostringstream os;
        os << "file entry named '" << name << "' not found";
        throw zip_error(os.str());
    }

    return get_file_entry_header(it->second);
}

void zip_archive::impl::read_file_entries()
{
    m_file_params.clear();

    zip_stream_parser central_dir(m_stream, m_central_dir_pos);
    uint32_t magic_num = central_dir.read_4bytes();

    while (magic_num == 0x02014b50)
    {
        zip_file_param param;

        param.version_made_by = central_dir.read_2bytes();
        param.minimum_version_needed = central_dir.read_2bytes();
        param.flags = central_dir.read_2bytes();
        param.compress_method =
            static_cast<zip_file_param::compress_method_type>(central_dir.read_2bytes());

        param.last_modified_time = central_dir.read_2bytes();
        param.last_modified_date = central_dir.read_2bytes();
        param.crc32 = central_dir.read_4bytes();
        param.size_compressed = central_dir.read_4bytes();
        param.size_uncompressed = central_dir.read_4bytes();
        param.filename_length = central_dir.read_2bytes();
        param.extra_field_length = central_dir.read_2bytes();
        param.comment_length = central_dir.read_2bytes();
        param.disk_id_where_file_starts = central_dir.read_2bytes();
        param.file_attributes_internal = central_dir.read_2bytes();
        param.file_attributes_external = central_dir.read_4bytes();
        param.offset_file_header = central_dir.read_4bytes();

        if (param.filename_length)
            param.filename = central_dir.read_string(param.filename_length, m_pool);

        if (param.extra_field_length)
            // Ignore extra field for now.
            central_dir.skip_bytes(param.extra_field_length);

        if (param.comment_length)
            // Ignore file comment for now.
            central_dir.skip_bytes(param.comment_length);

        magic_num = central_dir.read_4bytes(); // magic number for the next entry.

        m_file_params.push_back(param);
        m_filenames.insert(filename_map_type::value_type(param.filename, m_file_params.size()-1));

#if ORCUS_DEBUG_ZIP_ARCHIVE
        std::cout << "-- file entries" << std::endl;
        printf( "  magic number: 0x%8.8x\n", magic_num);
        std::cout << "  version made by: " << param.version_made_by << std::endl;
        std::cout << "  minimum version needed to extract: " << param.minimum_version_needed << std::endl;
        printf( "  general purpose bit flag: 0x%4.4x\n", param.flags);
        std::cout << "  compression method: " << param.compress_method << " (0=stored, 8=deflated)" << std::endl;
        std::cout << "  file last modified time: " << param.last_modified_time << std::endl;
        std::cout << "  file last modified date: " << param.last_modified_date << std::endl;
        printf( "  crc32: 0x%8.8x\n", param.crc32);
        std::cout << "  compressed size: " << param.size_compressed << std::endl;
        std::cout << "  uncompressed size: " << param.size_uncompressed << std::endl;
        std::cout << "  file name length: " << param.filename_length << std::endl;
        std::cout << "  extra field length: " << param.extra_field_length << std::endl;
        std::cout << "  file comment length: " << param.comment_length << std::endl;
        std::cout << "  disk number where file starts: " << param.disk_id_where_file_starts << std::endl;
        printf( "  internal file attributes: 0x%4.4x\n", param.file_attributes_internal);
        printf( "  external file attributes: 0x%8.8x\n", param.file_attributes_external);
        std::cout << "  relative offset of local file header: " << param.offset_file_header << std::endl;

        if (param.filename_length)
            std::cout << "  filename: '" << param.filename << "'" << std::endl;

        std::cout << "--" << std::endl;
#endif
    }
}

std::string_view zip_archive::impl::get_file_entry_name(std::size_t pos) const
{
    if (pos >= m_file_params.size())
        return std::string_view{};

    return m_file_params[pos].filename;
}

std::vector<unsigned char> zip_archive::impl::read_file_entry(std::string_view entry_name) const
{
    filename_map_type::const_iterator it = m_filenames.find(entry_name);
    if (it == m_filenames.end())
    {
        std::ostringstream os;
        os << "entry named '" << entry_name << "' not found";
        throw zip_error(os.str());
    }


    size_t index = it->second;
    if (index >= m_file_params.size())
        throw zip_error("entry index is out-of-bound");

    const zip_file_param& param = m_file_params[index];

    // Skip the file header section.
    zip_stream_parser file_header(m_stream, param.offset_file_header);
    file_header.skip_bytes(4);
    file_header.skip_bytes(2);
    file_header.skip_bytes(2);
    file_header.skip_bytes(2);
    file_header.skip_bytes(2);
    file_header.skip_bytes(2);
    file_header.skip_bytes(4);
    file_header.skip_bytes(4);
    file_header.skip_bytes(4);
    uint16_t filename_len = file_header.read_2bytes();
    uint16_t extra_field_len = file_header.read_2bytes();
    file_header.skip_bytes(filename_len);
    file_header.skip_bytes(extra_field_len);

    // Data section is immediately followed by the header section.
    m_stream->seek(file_header.tell());

    std::vector<unsigned char> raw_buf(param.size_compressed+1, 0);
    m_stream->read(raw_buf.data(), param.size_compressed);

    switch (param.compress_method)
    {
        case zip_file_param::stored:
        {
            // Not compressed at all.
            return raw_buf;
        }
        case zip_file_param::deflated:
        {
            // deflate compression
            std::vector<unsigned char> zip_buf(param.size_uncompressed+1, 0); // null-terminated
            zip_inflater inflater(raw_buf, zip_buf, param);
            if (!inflater.init())
                throw zip_error("error during initialization of inflater");

            if (!inflater.inflate())
                throw zip_error("error during inflate.");

            return zip_buf;
        }
    }

    throw std::logic_error("compress method can be either 'stored' or 'deflated', but neither has happened");
}

size_t zip_archive::impl::seek_central_dir()
{
    // Search for the position of 0x06054b50 (read in little endian order - so
    // it's 0x50, 0x4b, 0x05, 0x06 in this order) somewhere near the end of
    // the stream.

    unsigned char magic[] = { 0x06, 0x05, 0x4b, 0x50 };
    size_t n_magic = 4;

    off_t max_comment_size = 0xffff;

    size_t buf_size = 22 + max_comment_size; // central directory size is 22 + n (n maxing at 0xffff).
    std::vector<unsigned char> buf(buf_size);

    // Read stream backward and try to find the magic number.

    size_t read_end_pos = m_stream_size;
    while (read_end_pos)
    {
        if (read_end_pos < buf.size())
            // Last segment to read.
            buf.resize(read_end_pos);

        size_t read_pos = read_end_pos - buf.size();
        m_stream->seek(read_pos);
        m_stream->read(&buf[0], buf.size());

        // Search this byte segment for the magic number.
        std::vector<unsigned char>::reverse_iterator i = buf.rbegin(), ie = buf.rend();
        size_t magic_pos = 0;
        for (; i != ie; ++i)
        {
            // 06 05 4b 50
            if (*i == magic[magic_pos])
            {
                ++magic_pos;
                if (magic_pos == n_magic)
                {
                    // magic number is found.
                    size_t dist = distance(buf.rbegin(), i) + 1;
                    size_t pos = read_end_pos - dist;
                    return pos;
                }
            }
            else
                magic_pos = 0;
        }

        read_end_pos -= buf.size();
    }

    return 0;
}

void zip_archive::impl::read_central_dir_end()
{
    central_dir_end content;
    content.magic_number = m_central_dir_end.read_4bytes();
    content.this_disk_id = m_central_dir_end.read_2bytes();
    content.central_dir_disk_id = m_central_dir_end.read_2bytes();
    content.num_central_dir_records_local = m_central_dir_end.read_2bytes();
    content.num_celtral_dir_records_total = m_central_dir_end.read_2bytes();
    content.size_central_dir = m_central_dir_end.read_4bytes();
    content.central_dir_pos = m_central_dir_end.read_4bytes();
    m_central_dir_pos = content.central_dir_pos;

    content.comment_length = m_central_dir_end.read_2bytes();

#if ORCUS_DEBUG_ZIP_ARCHIVE
    std::cout << "-- central directory content" << std::endl;
    printf("  magic number: 0x%8.8x\n", content.magic_number);
    std::cout << "  number of this disk: " << content.this_disk_id << std::endl;
    std::cout << "  disk where central directory starts: " << content.central_dir_disk_id << std::endl;
    std::cout << "  number of central directory records on this disk: " << content.num_central_dir_records_local << std::endl;
    std::cout << "  total number of central directory records: " << content.num_celtral_dir_records_total << std::endl;
    std::cout << "  size of central directory: " << content.size_central_dir << std::endl;
    std::cout << "  offset of start of central directory, relative to start of archive: " << content.central_dir_pos << std::endl;
    std::cout << "  comment length: " << content.comment_length << std::endl;
    std::cout << "--" << std::endl;
#endif
}

zip_archive::zip_archive(zip_archive_stream* stream) : mp_impl(std::make_unique<impl>(stream))
{
}

zip_archive::~zip_archive() = default;

void zip_archive::load()
{
    mp_impl->load();
}

zip_file_entry_header zip_archive::get_file_entry_header(std::size_t index) const
{
    return mp_impl->get_file_entry_header(index);
}

zip_file_entry_header zip_archive::get_file_entry_header(std::string_view name) const
{
    return mp_impl->get_file_entry_header(name);
}

std::string_view zip_archive::get_file_entry_name(std::size_t index) const
{
    return mp_impl->get_file_entry_name(index);
}

size_t zip_archive::get_file_entry_count() const
{
    return mp_impl->get_file_entry_count();
}

std::vector<unsigned char> zip_archive::read_file_entry(std::string_view entry_name) const
{
    return mp_impl->read_file_entry(entry_name);
}

}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
