
#include <orcus/spreadsheet/import_interface.hpp>
#include <orcus/orcus_ods.hpp>

#include <iostream>

using namespace std;
using namespace orcus::spreadsheet;
using orcus::orcus_ods;

class my_import_sheet : public iface::import_sheet
{
public:
    virtual void set_auto(row_t row, col_t col, const char* p, size_t n) override
    {
    }

    virtual void set_string(row_t row, col_t col, size_t sindex) override
    {
    }

    virtual void set_value(row_t row, col_t col, double value) override
    {
    }

    virtual void set_bool(row_t row, col_t col, bool value) override
    {
    }

    virtual void set_date_time(row_t row, col_t col, int year, int month, int day, int hour, int minute, double second) override
    {
    }

    virtual void set_format(row_t row, col_t col, size_t xf_index) override
    {
    }

    virtual void set_format(row_t row_start, col_t col_start, row_t row_end, col_t col_end, size_t xf_index) override
    {
    }

    virtual void set_formula(row_t row, col_t col, formula_grammar_t grammar, const char* p, size_t n) override
    {
    }

    virtual void set_shared_formula(row_t row, col_t col, formula_grammar_t grammar, size_t sindex, const char* p_formula, size_t n_formula) override
    {
    }

    virtual void set_shared_formula(row_t row, col_t col, size_t sindex) override
    {
    }

    virtual void set_formula_result(row_t row, col_t col, double value) override
    {
    }

    virtual void set_formula_result(row_t row, col_t col, const char* p, size_t n) override
    {
    }

    virtual iface::import_formula_result* set_array_formula(
        const range_t&, formula_grammar_t grammar, const char* p, size_t n) override
    {
        return nullptr;
    }

    virtual range_size_t get_sheet_size() const override
    {
        range_size_t ret;
        ret.columns = ret.rows = 0;
        return ret;
    }
};

class my_import_factory : public iface::import_factory
{
public:
    virtual ~my_import_factory() {}

    virtual iface::import_sheet* append_sheet(
        sheet_t sheet_index, const char* sheet_name, size_t sheet_name_length) override
    {
        cout << "append_sheet: sheet index: " << sheet_index
             << "; sheet name: " << string(sheet_name, sheet_name_length)
             << endl;
        return nullptr;
    }

    virtual iface::import_sheet* get_sheet(
        const char* sheet_name, size_t sheet_name_length) override
    {
        cout << "get_sheet: sheet name: "
             << string(sheet_name, sheet_name_length) << endl;
        return nullptr;
    }

    virtual iface::import_sheet* get_sheet(sheet_t sheet_index) override
    {
        cout << "get_sheet: sheet index: " << sheet_index << endl;
        return nullptr;
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
