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
#include <orcus/exception.hpp>

#include <filesystem>

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
namespace fs = std::filesystem;

void test_zip_archive_stream(zip_archive_stream* const strm, const uint8_t* const data, std::size_t const length)
{
    assert(strm->size() == length);
    assert(strm->tell() == 0);

    std::vector<uint8_t> buffer(length, 0);
    uint8_t* buf = buffer.data();

    strm->read({buf, 2});
    assert(std::equal(data, data + 2, buf));
    assert(strm->tell() == 0);
    strm->read({buf, length});
    assert(std::equal(data, data + length, buf));
    ASSERT_THROW(strm->read({buf, length + 1}));
    strm->read({buf, 0});

    strm->seek(2);
    assert(strm->tell() == 2);
    strm->read({buf, 2});
    assert(std::equal(data + 2, data + 4, buf));
    strm->seek(length);
    assert(strm->tell() == length);
    ASSERT_THROW(strm->seek(length + 1));
    assert(strm->tell() == length);
}

void test_zip_archive_stream_blob()
{
    ORCUS_TEST_FUNC_SCOPE;

    const uint8_t data[] = "My hovercraft is full of eels.";
    zip_archive_stream_blob strm(std::span{data});
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

namespace {

// Build a minimal valid zip blob containing a single zero-byte central
// directory entry with the given filename.  No local file header is
// emitted; load() only walks the central directory.
std::vector<uint8_t> make_zip_with_entry_name(std::string_view name)
{
    std::vector<uint8_t> buf;

    auto write_u16 = [&](uint16_t v) {
        buf.push_back(static_cast<uint8_t>(v));
        buf.push_back(static_cast<uint8_t>(v >> 8));
    };
    auto write_u32 = [&](uint32_t v) {
        buf.push_back(static_cast<uint8_t>(v));
        buf.push_back(static_cast<uint8_t>(v >> 8));
        buf.push_back(static_cast<uint8_t>(v >> 16));
        buf.push_back(static_cast<uint8_t>(v >> 24));
    };

    uint32_t cd_offset = 0;

    // Central directory entry.
    write_u32(0x02014b50);  // signature
    write_u16(0);  // version made by
    write_u16(0);  // min version needed
    write_u16(0);  // general purpose flags
    write_u16(0);  // compression method (stored)
    write_u16(0);  // last mod time
    write_u16(0);  // last mod date
    write_u32(0);  // crc32
    write_u32(0);  // compressed size
    write_u32(0);  // uncompressed size
    write_u16(static_cast<uint16_t>(name.size()));  // filename length
    write_u16(0);  // extra field length
    write_u16(0);  // file comment length
    write_u16(0);  // disk number where file starts
    write_u16(0);  // internal file attributes
    write_u32(0);  // external file attributes
    write_u32(0);  // local header offset
    buf.insert(buf.end(), name.begin(), name.end());

    uint32_t cd_size = static_cast<uint32_t>(buf.size() - cd_offset);

    // End of central directory.
    write_u32(0x06054b50);
    write_u16(0);  // this disk
    write_u16(0);  // disk where CD starts
    write_u16(1);  // entries on this disk
    write_u16(1);  // total entries
    write_u32(cd_size);
    write_u32(cd_offset);
    write_u16(0);  // archive comment length

    return buf;
}

// Build a zip blob with one entry whose local file header declares
// local_name while the central directory declares central_name, so the two
// filename copies can be made to diverge.  The local header sits at offset 0
// and the central directory entry points at it.
std::vector<uint8_t> make_zip_with_divergent_names(
    std::string_view central_name, std::string_view local_name)
{
    std::vector<uint8_t> buf;

    auto write_u16 = [&](uint16_t v) {
        buf.push_back(static_cast<uint8_t>(v));
        buf.push_back(static_cast<uint8_t>(v >> 8));
    };
    auto write_u32 = [&](uint32_t v) {
        buf.push_back(static_cast<uint8_t>(v));
        buf.push_back(static_cast<uint8_t>(v >> 8));
        buf.push_back(static_cast<uint8_t>(v >> 16));
        buf.push_back(static_cast<uint8_t>(v >> 24));
    };

    // Local file header at offset 0.
    const uint32_t local_header_offset = 0;
    write_u32(0x04034b50);  // signature
    write_u16(0);  // version needed
    write_u16(0);  // general purpose flags
    write_u16(0);  // compression method (stored)
    write_u16(0);  // last mod time
    write_u16(0);  // last mod date
    write_u32(0);  // crc32
    write_u32(0);  // compressed size
    write_u32(0);  // uncompressed size
    write_u16(static_cast<uint16_t>(local_name.size()));  // filename length
    write_u16(0);  // extra field length
    buf.insert(buf.end(), local_name.begin(), local_name.end());

    const uint32_t cd_offset = static_cast<uint32_t>(buf.size());

    // Central directory entry.
    write_u32(0x02014b50);  // signature
    write_u16(0);  // version made by
    write_u16(0);  // min version needed
    write_u16(0);  // general purpose flags
    write_u16(0);  // compression method (stored)
    write_u16(0);  // last mod time
    write_u16(0);  // last mod date
    write_u32(0);  // crc32
    write_u32(0);  // compressed size
    write_u32(0);  // uncompressed size
    write_u16(static_cast<uint16_t>(central_name.size()));  // filename length
    write_u16(0);  // extra field length
    write_u16(0);  // file comment length
    write_u16(0);  // disk number where file starts
    write_u16(0);  // internal file attributes
    write_u32(0);  // external file attributes
    write_u32(local_header_offset);  // local header offset
    buf.insert(buf.end(), central_name.begin(), central_name.end());

    const uint32_t cd_size = static_cast<uint32_t>(buf.size() - cd_offset);

    // End of central directory.
    write_u32(0x06054b50);
    write_u16(0);  // this disk
    write_u16(0);  // disk where CD starts
    write_u16(1);  // entries on this disk
    write_u16(1);  // total entries
    write_u32(cd_size);
    write_u32(cd_offset);
    write_u16(0);  // archive comment length

    return buf;
}

} // namespace

void test_zip_rejects_unsafe_entry_names()
{
    ORCUS_TEST_FUNC_SCOPE;

    // Real OOXML / ODF archives never carry these as entry names
    const std::string_view bad_names[] = {
        "../etc/anything",
        "/etc/anything",
        "foo/../bar",
        "..",
        "foo\\bar",
        std::string_view{"foo\0bar", 7},
    };

    for (auto name : bad_names)
    {
        std::vector<uint8_t> blob = make_zip_with_entry_name(name);
        zip_archive_stream_blob strm(std::span{blob.data(), blob.size()});
        zip_archive archive(&strm);
        bool threw = false;
        try
        {
            archive.load();
        }
        catch (const zip_error&)
        {
            threw = true;
        }
        assert(threw);
    }

    // Sanity check: a normal entry name still loads.
    std::vector<uint8_t> blob = make_zip_with_entry_name("xl/sharedStrings.xml");
    zip_archive_stream_blob strm(std::span{blob.data(), blob.size()});
    zip_archive archive(&strm);
    archive.load();
    assert(archive.get_file_entry_count() == 1);
    assert(archive.get_file_entry_name(0) == "xl/sharedStrings.xml");
}

namespace {

// Offsets within a make_zip_with_entry_name blob. The single central
// directory entry sits at byte 0; the EOCD follows the entry + the
// name. Used by the tests to set specific 4-byte fields.
struct zip_layout
{
    std::size_t entry_gp_flags;            // CD entry, byte 8
    std::size_t entry_disk_id;             // CD entry, byte 34
    std::size_t entry_local_header_offset; // CD entry, byte 42
    std::size_t eocd_this_disk;            // EOCD, byte +4 from sig
    std::size_t eocd_central_dir_disk;     // EOCD, byte +6 from sig
    std::size_t eocd_size_central_dir;     // EOCD, byte +12 from sig
    std::size_t eocd_central_dir_pos;      // EOCD, byte +16 from sig
    std::size_t eocd_total_entries;        // EOCD, byte +10 from sig
};

zip_layout zip_layout_for(std::string_view name)
{
    const std::size_t cd_entry_size = 46 + name.size();
    const std::size_t eocd_sig_pos = cd_entry_size;
    return {
        /* entry_gp_flags */ 8,
        /* entry_disk_id */ 34,
        /* entry_local_header_offset */ 42,
        /* eocd_this_disk */ eocd_sig_pos + 4,
        /* eocd_central_dir_disk */ eocd_sig_pos + 6,
        /* eocd_size_central_dir */ eocd_sig_pos + 12,
        /* eocd_central_dir_pos */ eocd_sig_pos + 16,
        /* eocd_total_entries */ eocd_sig_pos + 10,
    };
}

void poke_u32(std::vector<uint8_t>& buf, std::size_t off, uint32_t v)
{
    buf[off] = static_cast<uint8_t>(v);
    buf[off + 1] = static_cast<uint8_t>(v >> 8);
    buf[off + 2] = static_cast<uint8_t>(v >> 16);
    buf[off + 3] = static_cast<uint8_t>(v >> 24);
}

void poke_u16(std::vector<uint8_t>& buf, std::size_t off, uint16_t v)
{
    buf[off] = static_cast<uint8_t>(v);
    buf[off + 1] = static_cast<uint8_t>(v >> 8);
}

bool load_throws(const std::vector<uint8_t>& blob)
{
    zip_archive_stream_blob strm(std::span{blob.data(), blob.size()});
    zip_archive archive(&strm);
    try
    {
        archive.load();
    }
    catch (const zip_error&)
    {
        return true;
    }
    return false;
}

} // namespace

void test_zip_central_dir_bounds()
{
    ORCUS_TEST_FUNC_SCOPE;

    const std::string_view name = "a";
    const zip_layout L = zip_layout_for(name);

    // ZIP64 sentinel in central_dir_pos.
    {
        auto blob = make_zip_with_entry_name(name);
        poke_u32(blob, L.eocd_central_dir_pos, 0xFFFFFFFFu);
        assert(load_throws(blob));
    }

    // ZIP64 sentinel in size_central_dir.
    {
        auto blob = make_zip_with_entry_name(name);
        poke_u32(blob, L.eocd_size_central_dir, 0xFFFFFFFFu);
        assert(load_throws(blob));
    }

    // central_dir_pos points past the end of the archive.
    {
        auto blob = make_zip_with_entry_name(name);
        poke_u32(blob, L.eocd_central_dir_pos,
                 static_cast<uint32_t>(blob.size() + 1));
        assert(load_throws(blob));
    }

    // size_central_dir overruns the archive when added to central_dir_pos.
    {
        auto blob = make_zip_with_entry_name(name);
        poke_u32(blob, L.eocd_size_central_dir,
                 static_cast<uint32_t>(blob.size() + 1));
        assert(load_throws(blob));
    }

    // ZIP64 sentinel in a per-entry local-header offset.
    {
        auto blob = make_zip_with_entry_name(name);
        poke_u32(blob, L.entry_local_header_offset, 0xFFFFFFFFu);
        assert(load_throws(blob));
    }

    // Per-entry local-header offset past end of archive.
    {
        auto blob = make_zip_with_entry_name(name);
        poke_u32(blob, L.entry_local_header_offset,
                 static_cast<uint32_t>(blob.size()));
        assert(load_throws(blob));
    }
}

void test_zip_entry_count_cap()
{
    ORCUS_TEST_FUNC_SCOPE;

    // The CD parsing loop must stop after num_total_entries entries even
    // if the next 4 bytes happen to form a valid CD signature.  Declare
    // total_entries=0 and confirm the existing single entry is skipped.
    const std::string_view name = "a";
    const zip_layout L = zip_layout_for(name);
    auto blob = make_zip_with_entry_name(name);
    poke_u16(blob, L.eocd_total_entries, 0);

    zip_archive_stream_blob strm(std::span{blob.data(), blob.size()});
    zip_archive archive(&strm);
    archive.load();
    assert(archive.get_file_entry_count() == 0);
}

void test_zip_rejects_encrypted_and_multidisk()
{
    ORCUS_TEST_FUNC_SCOPE;

    const std::string_view name = "a";
    const zip_layout L = zip_layout_for(name);

    // General-purpose bit 0 set: an encrypted entry orcus cannot decrypt.
    {
        auto blob = make_zip_with_entry_name(name);
        poke_u16(blob, L.entry_gp_flags, 0x0001);
        assert(load_throws(blob));
    }

    // Per-entry disk id non-zero: payload lives on another volume.
    {
        auto blob = make_zip_with_entry_name(name);
        poke_u16(blob, L.entry_disk_id, 1);
        assert(load_throws(blob));
    }

    // EOCD "this disk" non-zero: a split archive.
    {
        auto blob = make_zip_with_entry_name(name);
        poke_u16(blob, L.eocd_this_disk, 1);
        assert(load_throws(blob));
    }

    // EOCD "disk where CD starts" non-zero: a split archive.
    {
        auto blob = make_zip_with_entry_name(name);
        poke_u16(blob, L.eocd_central_dir_disk, 1);
        assert(load_throws(blob));
    }
}

void test_zip_rejects_unsafe_local_header_name()
{
    ORCUS_TEST_FUNC_SCOPE;

    // The central directory carries a safe name, so load() succeeds, but the
    // local file header that get_file_entry_header reads declares an unsafe
    // one. The header read must reject it rather than hand back the
    // unsanitised (or NUL-truncated) name.
    const std::string_view bad_local_names[] = {
        "../etc/something",
        "/etc/something",
        "foo\\bar",
        std::string_view{"safe.xml\0../etc/something", 25},
    };

    for (auto local_name : bad_local_names)
    {
        auto blob = make_zip_with_divergent_names("safe.xml", local_name);
        zip_archive_stream_blob strm(std::span{blob.data(), blob.size()});
        zip_archive archive(&strm);
        archive.load();
        bool threw = false;
        try
        {
            archive.get_file_entry_header(std::size_t(0));
        }
        catch (const zip_error&)
        {
            threw = true;
        }
        assert(threw);
    }

    // Sanity check: matching safe names read back cleanly.
    auto blob = make_zip_with_divergent_names("safe.xml", "safe.xml");
    zip_archive_stream_blob strm(std::span{blob.data(), blob.size()});
    zip_archive archive(&strm);
    archive.load();
    zip_file_entry_header header = archive.get_file_entry_header(std::size_t(0));
    assert(header.filename == "safe.xml");
}

void test_seek_central_dir_window()
{
    ORCUS_TEST_FUNC_SCOPE;

    // EOCD records can only sit within the last (22 + 0xFFFF) = 65557
    // bytes of the archive per the ZIP spec.  Put a valid-looking EOCD
    // signature at byte 1 of a 70000-byte zero blob. That's outside
    // the last 65557 bytes, so load() should reject the archive
    // instead of finding the signature there.
    std::vector<uint8_t> blob(70000, 0);
    blob[1] = 0x50;
    blob[2] = 0x4b;
    blob[3] = 0x05;
    blob[4] = 0x06;

    zip_archive_stream_blob strm(std::span{blob.data(), blob.size()});
    zip_archive archive(&strm);
    bool threw = false;
    try
    {
        archive.load();
    }
    catch (const zip_error&)
    {
        threw = true;
    }
    assert(threw);
}

int main()
{
    test_zip_archive_stream_blob();
    test_zip_archive_file_entry_header();
    test_zip_rejects_unsafe_entry_names();
    test_zip_central_dir_bounds();
    test_zip_entry_count_cap();
    test_zip_rejects_encrypted_and_multidisk();
    test_zip_rejects_unsafe_local_header_name();
    test_seek_central_dir_window();

    return EXIT_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
