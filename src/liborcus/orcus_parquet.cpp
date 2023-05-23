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
    spreadsheet::iface::import_factory* m_factory;

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
        ss::iface::import_sheet* sheet = m_factory->append_sheet(0, sheet_name);

        if (!sheet)
            // Failed to append sheet. Bail out.
            return;

        const parquet::SchemaDescriptor* schema_desc = file_md->schema();

        std::vector<std::pair<ss::col_t, const parquet::ColumnDescriptor*>> column_types;
        column_types.reserve(schema_desc->num_columns());

        for (int i = 0; i < schema_desc->num_columns(); ++i)
        {
            const parquet::ColumnDescriptor* col_desc = schema_desc->Column(i);
            column_types.emplace_back(i, col_desc);
        }

        parquet::StreamReader stream{std::move(file_reader)};

        if (stream.eof())
            return;

        ss::iface::import_shared_strings* sstrings = m_factory->get_shared_strings();

        // Import column labels as first row
        for (const auto& [col, p] : column_types)
        {
            if (!sstrings)
                continue;

            std::size_t si = sstrings->add(p->name());
            sheet->set_string(0, col, si);
        }

        for (int i = 0; i < file_md->num_rows(); ++i)
        {
            ss::row_t row = i + 1; // account for the header row

            for (const auto& [col, p] : column_types)
            {
                switch (p->physical_type())
                {
                    case parquet::Type::BYTE_ARRAY:
                    {
                        switch (p->converted_type())
                        {
                            case parquet::ConvertedType::UTF8:
                            {
                                if (!sstrings)
                                    break;

                                std::string v;
                                stream >> v;
                                std::size_t si = sstrings->add(v);
                                sheet->set_string(row, col, si);
                                break;
                            }
                            default:
                                throw std::runtime_error("WIP: unhandled converted type for BYTE_ARRAY");
                        }
                        break;
                    }
                    case parquet::Type::INT64:
                    {
                        switch (p->converted_type())
                        {
                            case parquet::ConvertedType::NONE:
                            {
                                int64_t v;
                                stream >> v;
                                sheet->set_value(row, col, v);
                                break;
                            }
                            default:
                                throw std::runtime_error("WIP: unhandled converted type for INT64");
                        }
                        break;
                    }
                    case parquet::Type::BOOLEAN:
                    {
                        if (p->converted_type() != parquet::ConvertedType::NONE)
                            throw std::runtime_error("WIP: unhandled covnerted type for BOOLEAN");

                        bool v;
                        stream >> v;
                        sheet->set_bool(row, col, v);
                        break;
                    }
                    case parquet::Type::DOUBLE:
                    {
                        if (p->converted_type() != parquet::ConvertedType::NONE)
                            throw std::runtime_error("WIP: unhandled covnerted type for DOUBLE");

                        double v;
                        stream >> v;
                        sheet->set_value(row, col, v);
                        break;
                    }
                    default:
                    {
                        std::ostringstream os;
                        os << "WIP: not handled type: physical=" << p->physical_type() << "; converted=" << p->converted_type();
                        throw std::runtime_error(os.str());
                    }
                }
            }

            stream >> parquet::EndRow;
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
