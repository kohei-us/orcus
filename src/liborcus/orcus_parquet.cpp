/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <orcus/orcus_parquet.hpp>
#include <orcus/stream.hpp>
#include <orcus/spreadsheet/types.hpp>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include <arrow/io/file.h>
#include <parquet/stream_reader.h>
#pragma GCC diagnostic pop

#include <boost/filesystem/path.hpp>

namespace ss = orcus::spreadsheet;
namespace fs = boost::filesystem;

namespace orcus {

class orcus_parquet::impl
{
    using columns_type = std::vector<std::pair<ss::col_t, const parquet::ColumnDescriptor*>>;

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

    void import_byte_array(ss::row_t row, ss::col_t col, const parquet::ColumnDescriptor* p)
    {
        switch (p->converted_type())
        {
            case parquet::ConvertedType::UTF8:
            {
                if (!m_sstrings)
                    break;

                std::string v;
                m_stream >> v;
                std::size_t si = m_sstrings->add(v);
                m_sheet->set_string(row, col, si);
                break;
            }
            default:
                throw std::runtime_error("WIP: unhandled converted type for BYTE_ARRAY");
        }
    }

    void import_fixed_len_byte_array(ss::row_t /*row*/, ss::col_t /*col*/, const parquet::ColumnDescriptor* /*p*/)
    {
        throw std::runtime_error("WIP: physical=FIXED_LEN_BYTE_ARRAY not handled yet");
    }

    void import_int32(ss::row_t row, ss::col_t col, const parquet::ColumnDescriptor* p)
    {
        switch (p->converted_type())
        {
            case parquet::ConvertedType::NONE:
            {
                int32_t v;
                m_stream >> v;
                m_sheet->set_value(row, col, v);
                break;
            }
            default:
                throw std::runtime_error("WIP: unhandled converted type for INT32");
        }
    }

    void import_int64(ss::row_t row, ss::col_t col, const parquet::ColumnDescriptor* p)
    {
        switch (p->converted_type())
        {
            case parquet::ConvertedType::NONE:
            {
                int64_t v;
                m_stream >> v;
                m_sheet->set_value(row, col, v);
                break;
            }
            default:
                throw std::runtime_error("WIP: unhandled converted type for INT64");
        }
    }

    void import_int96(ss::row_t /*row*/, ss::col_t /*col*/, const parquet::ColumnDescriptor* /*p*/)
    {
        throw std::runtime_error("WIP: physical=INT96 not handled yet");
    }

    void import_boolean(ss::row_t row, ss::col_t col, const parquet::ColumnDescriptor* p)
    {
        if (p->converted_type() != parquet::ConvertedType::NONE)
            throw std::runtime_error("WIP: unhandled covnerted type for BOOLEAN");

        bool v;
        m_stream >> v;
        m_sheet->set_bool(row, col, v);
    }

    void import_float(ss::row_t row, ss::col_t col, const parquet::ColumnDescriptor* p)
    {
        if (p->converted_type() != parquet::ConvertedType::NONE)
            throw std::runtime_error("WIP: unhandled covnerted type for FLOAT");

        float v;
        m_stream >> v;
        m_sheet->set_value(row, col, v);
    }

    void import_double(ss::row_t row, ss::col_t col, const parquet::ColumnDescriptor* p)
    {
        if (p->converted_type() != parquet::ConvertedType::NONE)
            throw std::runtime_error("WIP: unhandled covnerted type for DOUBLE");

        double v;
        m_stream >> v;
        m_sheet->set_value(row, col, v);
    }

public:
    impl(spreadsheet::iface::import_factory* factory) : m_factory(factory) {}

    void read_file(fs::path filepath)
    {
        std::shared_ptr<arrow::io::ReadableFile> infile;

        PARQUET_ASSIGN_OR_THROW(
            infile,
            arrow::io::ReadableFile::Open(filepath.string()));

        auto file_reader = parquet::ParquetFileReader::Open(infile);
        auto file_md = file_reader->metadata();

        if (file_md->num_rows() < 0 || file_md->num_columns() < 0)
            // Nothing to import. Bail out.
            return;

        auto sheet_name = filepath.stem().string();
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
                        throw std::runtime_error(os.str());
                    }
                }
            }

            m_stream >> parquet::EndRow;
        }
    }
};

orcus_parquet::orcus_parquet(spreadsheet::iface::import_factory* factory) :
    iface::import_filter(format_t::parquet),
    mp_impl(std::make_unique<impl>(factory))
{
}

orcus_parquet::~orcus_parquet() = default;

bool orcus_parquet::detect(const unsigned char* /*blob*/, size_t /*size*/)
{
    return false;
}

void orcus_parquet::read_file(std::string_view filepath)
{
    mp_impl->read_file(fs::path{std::string{filepath}});
}

void orcus_parquet::read_stream(std::string_view /*stream*/)
{
    // TODO : Parquet API doesn't seem to support reading from stream. Figure
    // out how to implement this.
}

std::string_view orcus_parquet::get_name() const
{
    return "parquet";
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
