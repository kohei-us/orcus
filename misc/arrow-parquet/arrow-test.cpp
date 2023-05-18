#include <arrow/io/file.h>
#include <parquet/stream_reader.h>

#include <iostream>
#include <memory>
#include <string>

using std::cout;
using std::cerr;
using std::endl;

int main(int argc, char** argv)
{
    std::shared_ptr<arrow::io::ReadableFile> infile;

    PARQUET_ASSIGN_OR_THROW(
        infile,
        arrow::io::ReadableFile::Open("test.parquet"));

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
            if (cc->has_dictionary_page())
                cout << "      dictionary page offset: " << cc->dictionary_page_offset() << endl;
            cout << "      has index page: " << cc->has_index_page() << endl;
        }
    }

    cout << "schema:" << endl;
    const parquet::SchemaDescriptor* p = r.schema();
    cout << "  name: " << p->name() << endl;
    cout << "  num-columns: " << p->num_columns() << endl;

    for (int i = 0; i < p->num_columns(); ++i)
    {
        cout << "column " << i << ":" << endl;
        const parquet::ColumnDescriptor* col_desc = p->Column(i);
        cout << "  name: " << col_desc->name() << endl;
        cout << "  physical type: " << col_desc->physical_type() << endl;
        cout << "  converted type: " << col_desc->converted_type() << endl;
        cout << "  type length: " << col_desc->type_length() << endl;
    }

    parquet::StreamReader stream{std::move(file_reader)};

    std::string c1, c2, c4;
    bool c3;

    cout << "row values:" << endl;

    if (!stream.eof())
    {
        for (int i = 0; i < r.num_rows(); ++i)
        {
            stream >> c1 >> c2 >> c3 >> c4 >> parquet::EndRow;
            cout << c1 << ' ' << c2 << ' ' << c3 << ' ' << c4 <<    endl;
        }
    }

    return EXIT_SUCCESS;
}
