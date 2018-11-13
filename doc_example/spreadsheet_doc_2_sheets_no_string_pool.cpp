
#include <orcus/spreadsheet/import_interface.hpp>
#include <orcus/orcus_ods.hpp>

#include <iostream>
#include <memory>

using namespace std;
using namespace orcus::spreadsheet;
using orcus::orcus_ods;

enum class cell_value_type { empty, numeric, string };

struct cell_value
{
    cell_value_type type;

    union
    {
        size_t s;
        double f;
    };

    cell_value() : type(cell_value_type::empty) {}
};

class my_sheet : public iface::import_sheet
{
    cell_value m_cells[100][1000];
    range_size_t m_sheet_size;
    sheet_t m_sheet_index;

public:
    my_sheet(sheet_t sheet_index) :
        m_sheet_index(sheet_index)
    {
        m_sheet_size.rows = 1000;
        m_sheet_size.columns = 100;
    }

    virtual void set_string(row_t row, col_t col, size_t sindex) override
    {
        cout << "(sheet: " << m_sheet_index << "; row: " << row << "; col: " << col << "): string index = " << sindex << endl;

        m_cells[col][row].type = cell_value_type::string;
        m_cells[col][row].s = sindex;
    }

    virtual void set_value(row_t row, col_t col, double value) override
    {
        cout << "(sheet: " << m_sheet_index << "; row: " << row << "; col: " << col << "): value = " << value << endl;

        m_cells[col][row].type = cell_value_type::numeric;
        m_cells[col][row].f = value;
    }

    virtual range_size_t get_sheet_size() const override
    {
        return m_sheet_size;
    }

    // We don't implement these methods for now.
    virtual void set_auto(row_t row, col_t col, const char* p, size_t n) override {}
    virtual void set_bool(row_t row, col_t col, bool value) override {}
    virtual void set_date_time(row_t row, col_t col, int year, int month, int day, int hour, int minute, double second) override {}
    virtual void set_format(row_t row, col_t col, size_t xf_index) override {}
    virtual void set_format(row_t row_start, col_t col_start, row_t row_end, col_t col_end, size_t xf_index) override {}
};

class my_import_factory : public iface::import_factory
{
    std::vector<std::unique_ptr<my_sheet>> m_sheets;

public:
    virtual iface::import_sheet* append_sheet(
        sheet_t sheet_index, const char* sheet_name, size_t sheet_name_length) override
    {
        m_sheets.push_back(std::make_unique<my_sheet>(m_sheets.size()));
        return m_sheets.back().get();
    }

    virtual iface::import_sheet* get_sheet(
        const char* sheet_name, size_t sheet_name_length) override
    {
        // TODO : implement this.
        return nullptr;
    }

    virtual iface::import_sheet* get_sheet(sheet_t sheet_index) override
    {
        sheet_t sheet_count = m_sheets.size();
        return sheet_index < sheet_count ? m_sheets[sheet_index].get() : nullptr;
    }

    virtual void finalize() override {}
};

int main()
{
    my_import_factory factory;
    orcus_ods loader(&factory);
    loader.read_file(SRCDIR"/doc_example/files/multi-sheets.ods");

    return EXIT_SUCCESS;
}
