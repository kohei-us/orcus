
#include <orcus/spreadsheet/import_interface.hpp>
#include <orcus/orcus_ods.hpp>

#include <iostream>
#include <memory>
#include <filesystem>

//!code-start: cell_value
namespace ss = orcus::spreadsheet;

enum class cell_value_type { empty, numeric, string };

struct cell_value
{
    cell_value_type type;

    union
    {
        std::size_t index;
        double f;
    };

    cell_value() : type(cell_value_type::empty) {}
};
//!code-end: cell_value

//!code-start: my_sheet
class my_sheet : public ss::iface::import_sheet
{
    cell_value m_cells[100][1000];
    ss::range_size_t m_sheet_size;
    ss::sheet_t m_sheet_index;

public:
    my_sheet(ss::sheet_t sheet_index) :
        m_sheet_index(sheet_index)
    {
        m_sheet_size.rows = 1000;
        m_sheet_size.columns = 100;
    }

    virtual void set_string(ss::row_t row, ss::col_t col, ss::string_id_t sindex) override
    {
        std::cout << "(sheet: " << m_sheet_index << "; row: " << row << "; col: " << col
            << "): string index = " << sindex << std::endl;

        m_cells[col][row].type = cell_value_type::string;
        m_cells[col][row].index = sindex;
    }

    virtual void set_value(ss::row_t row, ss::col_t col, double value) override
    {
        std::cout << "(sheet: " << m_sheet_index << "; row: " << row << "; col: " << col
            << "): value = " << value << std::endl;

        m_cells[col][row].type = cell_value_type::numeric;
        m_cells[col][row].f = value;
    }

    virtual ss::range_size_t get_sheet_size() const override
    {
        return m_sheet_size;
    }

    // We don't implement these methods for now.
    virtual void set_auto(ss::row_t, ss::col_t, std::string_view) override {}

    virtual void set_bool(ss::row_t, ss::col_t, bool) override {}

    virtual void set_date_time(ss::row_t, ss::col_t, int, int, int, int, int, double) override {}

    virtual void set_format(ss::row_t, ss::col_t, std::size_t) override {}

    virtual void set_format(ss::row_t, ss::col_t, ss::row_t, ss::col_t, std::size_t) override {}

    virtual void set_column_format(ss::col_t, ss::col_t, std::size_t) override {}

    virtual void set_row_format(ss::col_t, std::size_t) override {}

    virtual void fill_down_cells(ss::row_t, ss::col_t, ss::row_t) override {}
};
//!code-end: my_sheet

//!code-start: my_import_factory
class my_import_factory : public ss::iface::import_factory
{
    std::vector<std::unique_ptr<my_sheet>> m_sheets;

public:
    virtual ss::iface::import_sheet* append_sheet(ss::sheet_t, std::string_view) override
    {
        m_sheets.push_back(std::make_unique<my_sheet>(m_sheets.size()));
        return m_sheets.back().get();
    }

    virtual ss::iface::import_sheet* get_sheet(std::string_view) override
    {
        // TODO : implement this.
        return nullptr;
    }

    virtual ss::iface::import_sheet* get_sheet(ss::sheet_t sheet_index) override
    {
        ss::sheet_t sheet_count = m_sheets.size();
        return sheet_index < sheet_count ? m_sheets[sheet_index].get() : nullptr;
    }

    virtual void finalize() override {}
};
//!code-end: my_import_factory

int main()
{
    std::filesystem::path input_dir = std::getenv("INPUTDIR");
    auto filepath = input_dir / "multi-sheets.ods";

    my_import_factory factory;
    orcus::orcus_ods loader(&factory);
    loader.read_file(filepath.native());

    return EXIT_SUCCESS;
}
