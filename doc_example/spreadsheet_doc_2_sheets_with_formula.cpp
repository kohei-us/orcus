
#include <orcus/spreadsheet/import_interface.hpp>
#include <orcus/orcus_ods.hpp>

#include <iostream>
#include <memory>
#include <unordered_map>
#include <deque>

using namespace std;
using namespace orcus::spreadsheet;
using orcus::orcus_ods;

enum class cell_value_type { empty, numeric, string, formula }; // adding a formula type here

using ss_type = std::deque<std::string>;
using ss_hash_type = std::unordered_map<std::string_view, std::size_t>;

struct cell_value
{
    cell_value_type type;

    union
    {
        size_t index;  // either a string index or a formula index
        double f;
    };

    cell_value() : type(cell_value_type::empty) {}
};

class cell_grid
{
    cell_value m_cells[100][1000];
public:

    cell_value& operator()(row_t row, col_t col)
    {
        return m_cells[col][row];
    }
};

struct formula
{
    std::string expression;
    formula_grammar_t grammar;

    formula() : grammar(formula_grammar_t::unknown) {}
    formula(std::string _expression, formula_grammar_t _grammar) :
        expression(std::move(_expression)),
        grammar(_grammar) {}
};

class my_formula : public iface::import_formula
{
    sheet_t m_sheet_index;
    cell_grid& m_cells;
    std::vector<formula>& m_formula_store;

    row_t m_row;
    col_t m_col;
    formula m_formula;

public:
    my_formula(sheet_t sheet, cell_grid& cells, std::vector<formula>& formulas) :
        m_sheet_index(sheet),
        m_cells(cells),
        m_formula_store(formulas),
        m_row(0),
        m_col(0) {}

    virtual void set_position(row_t row, col_t col) override
    {
        m_row = row;
        m_col = col;
    }

    virtual void set_formula(formula_grammar_t grammar, const char* p, size_t n) override
    {
        m_formula.expression = std::string(p, n);
        m_formula.grammar = grammar;
    }

    virtual void set_shared_formula_index(size_t index) override {}

    virtual void set_result_string(const char* p, size_t n) override {}
    virtual void set_result_value(double value) override {}
    virtual void set_result_empty() override {}
    virtual void set_result_bool(bool value) override {}

    virtual void commit() override
    {
        cout << "(sheet: " << m_sheet_index << "; row: " << m_row << "; col: " << m_col << "): formula = "
             << m_formula.expression << " (" << m_formula.grammar << ")" << endl;

        size_t index = m_formula_store.size();
        m_cells(m_row, m_col).type = cell_value_type::formula;
        m_cells(m_row, m_col).index = index;
        m_formula_store.push_back(std::move(m_formula));
    }
};

class my_sheet : public iface::import_sheet
{
    cell_grid m_cells;
    std::vector<formula> m_formula_store;
    my_formula m_formula_iface;
    range_size_t m_sheet_size;
    sheet_t m_sheet_index;
    const ss_type& m_string_pool;

public:
    my_sheet(sheet_t sheet_index, const ss_type& string_pool) :
        m_formula_iface(sheet_index, m_cells, m_formula_store),
        m_sheet_index(sheet_index),
        m_string_pool(string_pool)
    {
        m_sheet_size.rows = 1000;
        m_sheet_size.columns = 100;
    }

    virtual void set_string(row_t row, col_t col, string_id_t sindex) override
    {
        cout << "(sheet: " << m_sheet_index << "; row: " << row << "; col: " << col
             << "): string index = " << sindex << " (" << m_string_pool[sindex] << ")" << endl;

        m_cells(row, col).type = cell_value_type::string;
        m_cells(row, col).index = sindex;
    }

    virtual void set_value(row_t row, col_t col, double value) override
    {
        cout << "(sheet: " << m_sheet_index << "; row: " << row << "; col: " << col
             << "): value = " << value << endl;

        m_cells(row, col).type = cell_value_type::numeric;
        m_cells(row, col).f = value;
    }

    virtual range_size_t get_sheet_size() const override
    {
        return m_sheet_size;
    }

    // We don't implement these methods for now.
    virtual void set_auto(row_t row, col_t col, std::string_view s) override {}
    virtual void set_bool(row_t row, col_t col, bool value) override {}
    virtual void set_date_time(row_t row, col_t col, int year, int month, int day, int hour, int minute, double second) override {}
    virtual void set_format(row_t row, col_t col, size_t xf_index) override {}
    virtual void set_format(
        row_t row_start, col_t col_start, row_t row_end, col_t col_end, size_t xf_index) override {}
    virtual void fill_down_cells(row_t src_row, col_t src_col, row_t range_size) override {}

    virtual iface::import_formula* get_formula() override
    {
        return &m_formula_iface;
    }
};

class my_shared_strings : public iface::import_shared_strings
{
    ss_hash_type m_ss_hash;
    ss_type& m_ss;
    std::string m_current_string;

public:
    my_shared_strings(ss_type& ss) : m_ss(ss) {}

    virtual size_t add(std::string_view s) override
    {
        auto it = m_ss_hash.find(s);
        if (it != m_ss_hash.end())
            // This string already exists in the pool.
            return it->second;

        // This is a brand-new string.
        return append(s);
    }

    virtual size_t append(std::string_view s) override
    {
        size_t string_index = m_ss.size();
        m_ss.emplace_back(s);
        m_ss_hash.emplace(s, string_index);

        return string_index;
    }

    // The following methods are for formatted text segments, which we ignore for now.
    virtual void set_segment_bold(bool b) override {}
    virtual void set_segment_font(size_t font_index) override {}
    virtual void set_segment_font_color(color_elem_t alpha, color_elem_t red, color_elem_t green, color_elem_t blue) override {}
    virtual void set_segment_font_name(std::string_view s) override {}
    virtual void set_segment_font_size(double point) override {}
    virtual void set_segment_italic(bool b) override {}

    virtual void append_segment(std::string_view s) override
    {
        m_current_string += s;
    }

    virtual size_t commit_segments() override
    {
        size_t string_index = m_ss.size();
        m_ss.push_back(std::move(m_current_string));

        const std::string& s = m_ss.back();
        std::string_view sv(s.data(), s.size());
        m_ss_hash.emplace(sv, string_index);

        return string_index;
    }
};

class my_import_factory : public iface::import_factory
{
    ss_type m_string_pool; // string pool to be shared everywhere.
    my_shared_strings m_shared_strings;
    std::vector<std::unique_ptr<my_sheet>> m_sheets;

public:
    my_import_factory() : m_shared_strings(m_string_pool) {}

    virtual iface::import_shared_strings* get_shared_strings() override
    {
        return &m_shared_strings;
    }

    virtual iface::import_sheet* append_sheet(
        sheet_t sheet_index, const char* sheet_name, size_t sheet_name_length) override
    {
        // Pass the string pool to each sheet instance.
        m_sheets.push_back(std::make_unique<my_sheet>(m_sheets.size(), m_string_pool));
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
