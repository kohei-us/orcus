/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <orcus/orcus_parquet.hpp>
#include <orcus/stream.hpp>
#include <orcus/config.hpp>
#include <orcus/spreadsheet/types.hpp>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include <arrow/io/file.h>
#include <arrow/io/memory.h>
#include <parquet/stream_reader.h>
#include <parquet/arrow/reader.h>
#pragma GCC diagnostic pop

#include <boost/filesystem/path.hpp>
#include <boost/filesystem.hpp>
#include <iostream>
#include <unordered_map>

namespace ss = orcus::spreadsheet;
namespace fs = boost::filesystem;

namespace orcus {

class orcus_parquet::impl
{
    using columns_type = std::vector<std::pair<ss::col_t, const parquet::ColumnDescriptor*>>;

    const config& m_config;

    ss::iface::import_factory* m_factory = nullptr;
    ss::iface::import_shared_strings* m_sstrings = nullptr;
    ss::iface::import_sheet* m_sheet = nullptr;

    columns_type m_columns;

    parquet::StreamReader m_stream;

    static columns_type init_columns(const parquet::FileMetaData& file_md)
    {
        columns_type columns;

        const parquet::SchemaDescriptor* schema_desc = file_md.schema();

        if (!schema_desc)
            return columns;

        columns.reserve(schema_desc->num_columns());

        for (int i = 0; i < schema_desc->num_columns(); ++i)
        {
            const parquet::ColumnDescriptor* col_desc = schema_desc->Column(i);
            columns.emplace_back(i, col_desc);
        }

        return columns;
    }

    void warn(std::string_view msg) const
    {
        if (!m_config.debug)
            return;

        std::cerr << "warning: " << msg << std::endl;
    }

    /**
     * Import column labels as the first row.
     */
    void import_column_labels()
    {
        // Import column labels as first row
        for (const auto& [col, p] : m_columns)
        {
            if (!m_sstrings)
                continue;

            std::size_t si = m_sstrings->add(p->name());
            m_sheet->set_string(0, col, si);
        }
    }

    template<typename T>
    std::optional<T> read_or_warn(ss::row_t row, ss::col_t col)
    {
        T v;

        try
        {
            m_stream >> v;
            return v;
        }
        catch (const std::exception& e)
        {
            std::ostringstream os;
            os << "failed to read a value for (row=" << row << "; col=" << col << "): " << e.what();
            warn(os.str());
        }

        return std::optional<T>{};
    }

    void import_byte_array(ss::row_t row, ss::col_t col, const parquet::ColumnDescriptor* p)
    {
        switch (p->converted_type())
        {
            case parquet::ConvertedType::UTF8:
            {
                if (!m_sstrings)
                {
                    m_stream.SkipColumns(1);
                    break;
                }

                if (auto v = read_or_warn<std::string>(row, col); v)
                {
                    std::size_t si = m_sstrings->add(*v);
                    m_sheet->set_string(row, col, si);
                }
                break;
            }
            default:
            {
                std::ostringstream os;
                os << "WIP: unhandled converted type for BYTE_ARRAY (converted="
                    << p->converted_type() << ")";
                warn(os.str());
                m_stream.SkipColumns(1);
            }
        }
    }

    void import_fixed_len_byte_array(ss::row_t /*row*/, ss::col_t /*col*/, const parquet::ColumnDescriptor* /*p*/)
    {
        warn("WIP: physical=FIXED_LEN_BYTE_ARRAY not handled yet");
        m_stream.SkipColumns(1);
    }

    void import_int32(ss::row_t row, ss::col_t col, const parquet::ColumnDescriptor* p)
    {
        switch (p->converted_type())
        {
            case parquet::ConvertedType::NONE:
            {
                if (auto v = read_or_warn<int32_t>(row, col); v)
                    m_sheet->set_value(row, col, *v);
                break;
            }
            default:
                warn("WIP: unhandled converted type for INT32");
                m_stream.SkipColumns(1);
        }
    }

    void import_int64(ss::row_t row, ss::col_t col, const parquet::ColumnDescriptor* p)
    {
        switch (p->converted_type())
        {
            case parquet::ConvertedType::NONE:
            {
                if (auto v = read_or_warn<int64_t>(row, col); v)
                    m_sheet->set_value(row, col, *v);
                break;
            }
            default:
                warn("WIP: unhandled converted type for INT64");
                m_stream.SkipColumns(1);
        }
    }

    void import_int96(ss::row_t /*row*/, ss::col_t /*col*/, const parquet::ColumnDescriptor* /*p*/)
    {
        warn("WIP: physical=INT96 not handled yet");
        m_stream.SkipColumns(1);
    }

    void import_boolean(ss::row_t row, ss::col_t col, const parquet::ColumnDescriptor* p)
    {
        if (p->converted_type() != parquet::ConvertedType::NONE)
        {
            warn("WIP: unhandled covnerted type for BOOLEAN");
            m_stream.SkipColumns(1);
            return;
        }

        if (auto v = read_or_warn<bool>(row, col); v)
            m_sheet->set_bool(row, col, *v);
    }

    void import_float(ss::row_t row, ss::col_t col, const parquet::ColumnDescriptor* p)
    {
        if (p->converted_type() != parquet::ConvertedType::NONE)
        {
            warn("WIP: unhandled covnerted type for FLOAT");
            m_stream.SkipColumns(1);
            return;
        }

        if (auto v = read_or_warn<float>(row, col); v)
            m_sheet->set_value(row, col, *v);
    }

    void import_double(ss::row_t row, ss::col_t col, const parquet::ColumnDescriptor* p)
    {
        if (p->converted_type() != parquet::ConvertedType::NONE)
        {
            warn("WIP: unhandled covnerted type for DOUBLE");
            m_stream.SkipColumns(1);
            return;
        }

        if (auto v = read_or_warn<double>(row, col); v)
            m_sheet->set_value(row, col, *v);
    }

    void dump_metadata(const parquet::FileMetaData& metadata) const
    {
        if (!m_config.debug)
            return;

        auto _bool_v = [](bool v) { return v ? "true" : "false"; };

        auto _version_v = [](parquet::ParquetVersion::type t) -> std::string
        {
            const std::unordered_map<parquet::ParquetVersion::type, std::string_view> mapping =
            {
                { parquet::ParquetVersion::PARQUET_1_0, "PARQUET_1_0" },
                { parquet::ParquetVersion::PARQUET_2_4, "PARQUET_2_4" },
                { parquet::ParquetVersion::PARQUET_2_6, "PARQUET_2_6" },
            };

            std::ostringstream os;
            auto it = mapping.find(t);
            os << (it == mapping.end() ? "???" : it->second) << " (" << int(t) << ")";
            return os.str();
        };

        auto _compression_v = [](parquet::Compression::type t) -> std::string
        {
            const std::unordered_map<parquet::Compression::type, std::string_view> mapping =
            {
                { parquet::Compression::UNCOMPRESSED, "UNCOMPRESSED" },
                { parquet::Compression::SNAPPY, "SNAPPY" },
                { parquet::Compression::GZIP, "GZIP" },
                { parquet::Compression::BROTLI, "BROTLI" },
                { parquet::Compression::ZSTD, "ZSTD" },
                { parquet::Compression::LZ4, "LZ4" },
                { parquet::Compression::LZ4_FRAME, "LZ4_FRAME" },
                { parquet::Compression::LZO, "LZO" },
                { parquet::Compression::BZ2, "BZ2" },
                { parquet::Compression::LZ4_HADOOP, "LZ4_HADOOP" },
            };

            std::ostringstream os;
            auto it = mapping.find(t);
            os << (it == mapping.end() ? "???" : it->second) << " (" << int(t) << ")";
            return os.str();
        };

        std::cerr << "metadata size: " << metadata.size() << std::endl;
        std::cerr << "version: " << _version_v(metadata.version()) << std::endl;
        std::cerr << "created by: " << metadata.created_by() << std::endl;
        std::cerr << "num columns: " << metadata.num_columns() << std::endl;
        std::cerr << "num rows: " << metadata.num_rows() << std::endl;
        std::cerr << "num row groups: " << metadata.num_row_groups() << std::endl;
        std::cerr << "num schema elements: " << metadata.num_schema_elements() << std::endl;
        std::cerr << "can decompress: " << _bool_v(metadata.can_decompress()) << std::endl;

        for (int i = 0; i < metadata.num_row_groups(); ++i)
        {
            std::cerr << "row group " << i << ":" << std::endl;
            auto rg = metadata.RowGroup(i);
            std::cerr << "  num rows: " << rg->num_rows() << std::endl;
            std::cerr << "  total byte size: " << rg->total_byte_size() << std::endl;
            std::cerr << "  total compressed size: " << rg->total_compressed_size() << std::endl;
            std::cerr << "  file offset: " << rg->file_offset() << std::endl;
            std::cerr << "  num columns: " << rg->num_columns() << std::endl;

            for (int j = 0; j < rg->num_columns(); ++j)
            {
                std::cerr << "  column chunk " << j << ":" << std::endl;
                auto cc = rg->ColumnChunk(j);
                std::cerr << "    file path: " << cc->file_path() << std::endl;
                std::cerr << "    num values: " << cc->num_values() << std::endl;
                std::cerr << "    type: " << cc->type() << std::endl;
                std::cerr << "    data page offset: " << std::dec << cc->data_page_offset() << std::endl;
                std::cerr << "    compression: " << _compression_v(cc->compression()) << std::endl;
                std::cerr << "    has dictionary page: " << _bool_v(cc->has_dictionary_page()) << std::endl;

                if (cc->has_dictionary_page())
                    std::cerr << "    dictionary page offset: " << cc->dictionary_page_offset() << std::endl;

                std::cerr << "    has index page: " << _bool_v(cc->has_index_page()) << std::endl;
                if (cc->has_index_page())
                    std::cerr << "    index page offset: " << cc->index_page_offset() << std::endl;
            }
        }

        if (const parquet::SchemaDescriptor* schema_desc = metadata.schema(); schema_desc)
        {
            std::cerr << "schema:" << std::endl;
            std::cerr << "  name: " << schema_desc->name() << std::endl;
            std::cerr << "  num columns: " << schema_desc->num_columns() << std::endl;

            for (int i = 0; i < schema_desc->num_columns(); ++i)
            {
                if (const parquet::ColumnDescriptor* col_desc = schema_desc->Column(i); col_desc)
                {
                    std::cerr << "  column " << i << ":" << std::endl;
                    std::cerr << "    name: " << col_desc->name() << std::endl;
                    std::cerr << "    physical type: " << col_desc->physical_type() << std::endl;
                    std::cerr << "    converted type: " << col_desc->converted_type() << std::endl;
                    std::cerr << "    type length: " << col_desc->type_length() << std::endl;
                }
            }
        }
    }

    void read_stream_with_sheet_name(std::string_view sheet_name, std::string_view stream)
    {
        auto buf = std::make_shared<arrow::io::BufferReader>(stream);

        auto file_reader = parquet::ParquetFileReader::Open(buf);
        if (!file_reader)
        {
            warn("failed to open a parquet file reader from an in-memory buffer.");
            return;
        }

        auto file_md = file_reader->metadata();

        dump_metadata(*file_md);

        if (file_md->num_rows() < 0 || file_md->num_columns() < 0)
            // Nothing to import. Bail out.
            return;

        m_sheet = m_factory->append_sheet(0, sheet_name);

        if (!m_sheet)
            // Failed to append sheet. Bail out.
            return;

        m_columns = init_columns(*file_md);
        if (m_columns.empty())
            // Column data initialization failed. Bail out.
            return;

        m_stream = parquet::StreamReader{std::move(file_reader)};
        if (m_stream.eof())
            return;

        m_sstrings = m_factory->get_shared_strings();

        import_column_labels();

        for (int i = 0; i < file_md->num_rows(); ++i)
        {
            ss::row_t row = i + 1; // account for the header row

            for (const auto& [col, p] : m_columns)
            {
                switch (p->physical_type())
                {
                    case parquet::Type::BOOLEAN:
                    {
                        import_boolean(row, col, p);
                        break;
                    }
                    case parquet::Type::INT32:
                    {
                        import_int32(row, col, p);
                        break;
                    }
                    case parquet::Type::INT64:
                    {
                        import_int64(row, col, p);
                        break;
                    }
                    case parquet::Type::INT96:
                    {
                        import_int96(row, col, p);
                        break;
                    }
                    case parquet::Type::FLOAT:
                    {
                        import_float(row, col, p);
                        break;
                    }
                    case parquet::Type::DOUBLE:
                    {
                        import_double(row, col, p);
                        break;
                    }
                    case parquet::Type::BYTE_ARRAY:
                    {
                        import_byte_array(row, col, p);
                        break;
                    }
                    case parquet::Type::FIXED_LEN_BYTE_ARRAY:
                    {
                        import_fixed_len_byte_array(row, col, p);
                        break;
                    }
                    default:
                    {
                        std::ostringstream os;
                        os << "WIP: type not handled: physical=" << p->physical_type() << "; converted=" << p->converted_type();
                        warn(os.str());
                        m_stream.SkipColumns(1);
                    }
                }
            }

            m_stream >> parquet::EndRow;
        }

        m_factory->finalize();
    }

public:
    impl(const config& c, ss::iface::import_factory* factory) : m_config(c), m_factory(factory) {}

    void read_file(fs::path filepath)
    {
        file_content fc(filepath.string());
        try
        {
            read_stream_with_sheet_name(filepath.stem().string(), fc.str());
        }
        catch (const std::exception& e)
        {
            warn(e.what());
            m_factory->finalize();
        }
    }

    void read_stream(std::string_view stream)
    {
        try
        {
            read_stream_with_sheet_name("Data", stream);
        }
        catch (const std::exception& e)
        {
            warn(e.what());
            m_factory->finalize();
        }
    }
};

orcus_parquet::orcus_parquet(spreadsheet::iface::import_factory* factory) :
    iface::import_filter(format_t::parquet),
    mp_impl(std::make_unique<impl>(get_config(), factory))
{
}

orcus_parquet::~orcus_parquet() = default;

bool orcus_parquet::detect(const unsigned char* blob, std::size_t size)
{
    if (size < 12u)
        // At minimum header magic bytes (4), footer magic bytes (4), and the
        // footer metadata length (4).
        return false;

    const auto* p = blob;

    // Check the first 4 bytes.
    if (std::string_view(reinterpret_cast<const char*>(p), 4) != "PAR1")
        return false;

    // Check the last 4 bytes.
    p += size - 4u;
    if (std::string_view(reinterpret_cast<const char*>(p), 4) != "PAR1")
        return false;

    // Check the footer metadata size (little endian)
    p -= 1u;
    std::uint32_t footer_size = *p--;
    footer_size <<= 8;
    footer_size |= *p--;
    footer_size <<= 8;
    footer_size |= *p--;
    footer_size <<= 8;
    footer_size |= *p;

    p -= footer_size;
    if (p <= blob)
        // footer metadata position must be within the stream.
        return false;

    return true;
}

void orcus_parquet::read_file(std::string_view filepath)
{
    mp_impl->read_file(fs::path{std::string{filepath}});
}

void orcus_parquet::read_stream(std::string_view stream)
{
    mp_impl->read_stream(stream);
}

std::string_view orcus_parquet::get_name() const
{
    return "parquet";
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
