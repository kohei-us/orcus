/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <orcus/orcus_parquet.hpp>
#include <orcus/stream.hpp>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include <arrow/io/file.h>
#include <parquet/stream_reader.h>
#pragma GCC diagnostic pop

#include <iostream>

using std::cout;
using std::endl;

namespace orcus {

struct orcus_parquet::impl
{
    spreadsheet::iface::import_factory* factory;

    impl(spreadsheet::iface::import_factory* _factory) : factory(_factory) {}
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
    std::shared_ptr<arrow::io::ReadableFile> infile;

    PARQUET_ASSIGN_OR_THROW(
        infile,
        arrow::io::ReadableFile::Open(std::string(filepath)));

    auto file_reader = parquet::ParquetFileReader::Open(infile);
    auto file_md = file_reader->metadata();

    const parquet::SchemaDescriptor* schema_desc = file_md->schema();

    std::vector<const parquet::ColumnDescriptor*> column_types;
    column_types.reserve(schema_desc->num_columns());

    for (int i = 0; i < schema_desc->num_columns(); ++i)
    {
        cout << "column " << i << ":" << endl;
        const parquet::ColumnDescriptor* col_desc = schema_desc->Column(i);
        column_types.push_back(col_desc);

        cout << "  name: " << col_desc->name() << endl;
        cout << "  physical type: " << col_desc->physical_type() << endl;
        cout << "  converted type: " << col_desc->converted_type() << endl;
        cout << "  type length: " << col_desc->type_length() << endl;
    }

    parquet::StreamReader stream{std::move(file_reader)};

    if (stream.eof())
        return;

    cout << "row values:" << endl;

    // print column labels
    for (const parquet::ColumnDescriptor* p : column_types)
        cout << p->name() << ' ';
    cout << endl;

    for (int i = 0; i < file_md->num_rows(); ++i)
    {
        for (const parquet::ColumnDescriptor* p : column_types)
        {
            switch (p->physical_type())
            {
                case parquet::Type::BYTE_ARRAY:
                {
                    switch (p->converted_type())
                    {
                        case parquet::ConvertedType::UTF8:
                        {
                            std::string v;
                            stream >> v;
                            cout << v << ' ';
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
                            cout << v << ' ';
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
                    cout << v << ' ';
                    break;
                }
                case parquet::Type::DOUBLE:
                {
                    if (p->converted_type() != parquet::ConvertedType::NONE)
                        throw std::runtime_error("WIP: unhandled covnerted type for DOUBLE");

                    double v;
                    stream >> v;
                    cout << v << ' ';
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
        cout << endl;
    }
}

void orcus_parquet::read_stream(std::string_view /*stream*/)
{
    // TODO : Parquet API doesn't seem to support reading from stream.
}

std::string_view orcus_parquet::get_name() const
{
    return "parquet";
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
