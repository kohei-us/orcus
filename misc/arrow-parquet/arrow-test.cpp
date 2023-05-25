#include <arrow/io/file.h>
#include <parquet/stream_reader.h>

#include <iostream>
#include <memory>
#include <string>
#include <sstream>

using std::cout;
using std::cerr;
using std::endl;

int main(int argc, char** argv)
{
    if (argc < 2)
        return EXIT_FAILURE;

    const char* filepath = argv[1];
    std::shared_ptr<arrow::io::ReadableFile> infile;

    PARQUET_ASSIGN_OR_THROW(
        infile,
        arrow::io::ReadableFile::Open(filepath));

    auto file_reader = parquet::ParquetFileReader::Open(infile);
    auto file_md = file_reader->metadata();
    const parquet::FileMetaData& r = *file_md;

    cout << "num-columns: " << r.num_columns() << endl;
    cout << "num-rows: " << r.num_rows() << endl;
    cout << "num-row-groups: " << r.num_row_groups() << endl;
    cout << "num-schema-elements: " << r.num_schema_elements() << endl;
    cout << "can-decompress: " << r.can_decompress() << endl;

    for (int i = 0; i < r.num_row_groups(); ++i)
    {
        cout << "row-group " << i << ":" << endl;
        auto rg = r.RowGroup(i);
        cout << "  num rows: " << rg->num_rows() << endl;
        cout << "  total byte size: " << rg->total_byte_size() << endl;
        cout << "  total compressed size: " << rg->total_compressed_size() << endl;
        cout << "  file offset: " << rg->file_offset() << endl;
        cout << "  num columns: " << rg->num_columns() << endl;

        for (int j = 0; j < rg->num_columns(); ++j)
        {
            cout << "    column chunk " << j << ":" << endl;
            auto cc = rg->ColumnChunk(j);
            cout << "      file path: " << cc->file_path() << endl;
            cout << "      num values: " << cc->num_values() << endl;
            cout << "      type: " << cc->type() << endl;
            cout << "      data page offset: " << std::dec << cc->data_page_offset() << endl;
            cout << "      has dictionary page: " << cc->has_dictionary_page() << endl;
            cout << "      compression: " << cc->compression() << endl;
            if (cc->has_dictionary_page())
                cout << "      dictionary page offset: " << cc->dictionary_page_offset() << endl;
            cout << "      has index page: " << cc->has_index_page() << endl;
        }
    }

    cout << "schema:" << endl;
    const parquet::SchemaDescriptor* p = r.schema();
    cout << "  name: " << p->name() << endl;
    cout << "  num-columns: " << p->num_columns() << endl;

    std::vector<const parquet::ColumnDescriptor*> column_types;
    column_types.reserve(p->num_columns());

    for (int i = 0; i < p->num_columns(); ++i)
    {
        cout << "column " << i << ":" << endl;
        const parquet::ColumnDescriptor* col_desc = p->Column(i);
        column_types.push_back(col_desc);

        cout << "  name: " << col_desc->name() << endl;
        cout << "  physical type: " << col_desc->physical_type() << endl;
        cout << "  converted type: " << col_desc->converted_type() << endl;
        cout << "  type length: " << col_desc->type_length() << endl;
    }

    parquet::StreamReader stream{std::move(file_reader)};

    if (stream.eof())
        return EXIT_SUCCESS;

    cout << "row values:" << endl;

    // print column labels
    for (const parquet::ColumnDescriptor* p : column_types)
        cout << p->name() << ' ';
    cout << endl;

    for (int i = 0; i < r.num_rows(); ++i)
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

    return EXIT_SUCCESS;
}
