
#include <orcus/spreadsheet/import_interface.hpp>
#include <orcus/orcus_ods.hpp>

#include <iostream>
#include <memory>
#include <unordered_map>
#include <deque>
#include <filesystem>

namespace ss = orcus::spreadsheet;

//!code-start: cell_value_type
enum class cell_value_type { empty, numeric, string, formula }; // adding a formula type here
//!code-end: cell_value_type

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

//!code-start: cell_grid
class cell_grid
{
    cell_value m_cells[100][1000];
public:

    cell_value& operator()(ss::row_t row, ss::col_t col)
    {
        return m_cells[col][row];
    }
};
//!code-end: cell_grid

//!code-start: formula
struct formula
{
    std::string expression;
    ss::formula_grammar_t grammar;

    formula() : grammar(ss::formula_grammar_t::unknown) {}
    formula(std::string _expression, ss::formula_grammar_t _grammar) :
        expression(std::move(_expression)),
        grammar(_grammar) {}
};
//!code-end: formula

//!code-start: my_formula
class my_formula : public ss::iface::import_formula
{
    ss::sheet_t m_sheet_index;
    cell_grid& m_cells;
    std::vector<formula>& m_formula_store;

    ss::row_t m_row;
    ss::col_t m_col;
    formula m_formula;

public:
    my_formula(ss::sheet_t sheet, cell_grid& cells, std::vector<formula>& formulas) :
        m_sheet_index(sheet),
        m_cells(cells),
        m_formula_store(formulas),
        m_row(0),
        m_col(0) {}

    virtual void set_position(ss::row_t row, ss::col_t col) override
    {
        m_row = row;
        m_col = col;
    }

    virtual void set_formula(ss::formula_grammar_t grammar, std::string_view formula) override
    {
        m_formula.expression = formula;
        m_formula.grammar = grammar;
    }

    virtual void set_shared_formula_index(std::size_t) override {}

    virtual void set_result_string(std::string_view) override {}

    virtual void set_result_value(double) override {}

    virtual void set_result_empty() override {}

    virtual void set_result_bool(bool) override {}

    virtual void commit() override
    {
        std::cout << "(sheet: " << m_sheet_index << "; row: " << m_row << "; col: " << m_col << "): formula = "
            << m_formula.expression << " (" << m_formula.grammar << ")" << std::endl;

        std::size_t index = m_formula_store.size();
        m_cells(m_row, m_col).type = cell_value_type::formula;
        m_cells(m_row, m_col).index = index;
        m_formula_store.push_back(std::move(m_formula));
    }
};
//!code-end: my_formula

//!code-start: my_sheet
class my_sheet : public ss::iface::import_sheet
{
    cell_grid m_cells;
    std::vector<formula> m_formula_store;
    my_formula m_formula_iface;
    ss::range_size_t m_sheet_size;
    ss::sheet_t m_sheet_index;
    const ss_type& m_string_pool;

public:
    my_sheet(ss::sheet_t sheet_index, const ss_type& string_pool) :
        m_formula_iface(sheet_index, m_cells, m_formula_store),
        m_sheet_index(sheet_index),
        m_string_pool(string_pool)
    {
        m_sheet_size.rows = 1000;
        m_sheet_size.columns = 100;
    }

    virtual void set_string(ss::row_t row, ss::col_t col, ss::string_id_t sindex) override
    {
        std::cout << "(sheet: " << m_sheet_index << "; row: " << row << "; col: " << col
             << "): string index = " << sindex << " (" << m_string_pool[sindex] << ")" << std::endl;

        m_cells(row, col).type = cell_value_type::string;
        m_cells(row, col).index = sindex;
    }

    virtual void set_value(ss::row_t row, ss::col_t col, double value) override
    {
        std::cout << "(sheet: " << m_sheet_index << "; row: " << row << "; col: " << col
             << "): value = " << value << std::endl;

        m_cells(row, col).type = cell_value_type::numeric;
        m_cells(row, col).f = value;
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

    virtual void set_column_format(ss::col_t, std::size_t) override {}

    virtual void set_row_format(ss::col_t, std::size_t) override {}

    virtual void fill_down_cells(ss::row_t, ss::col_t, ss::row_t) override {}

    virtual ss::iface::import_formula* get_formula() override
    {
        return &m_formula_iface;
    }
};
//!code-end: my_sheet

class my_shared_strings : public ss::iface::import_shared_strings
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
        std::size_t string_index = m_ss.size();
        m_ss.emplace_back(s);
        m_ss_hash.emplace(s, string_index);

        return string_index;
    }

    // The following methods are for formatted text segments, which we ignore for now.
    virtual void set_segment_bold(bool) override {}

    virtual void set_segment_font(std::size_t) override {}

    virtual void set_segment_font_color(
        ss::color_elem_t,
        ss::color_elem_t,
        ss::color_elem_t,
        ss::color_elem_t) override {}

    virtual void set_segment_font_name(std::string_view) override {}

    virtual void set_segment_font_size(double) override {}

    virtual void set_segment_italic(bool) override {}

    virtual void append_segment(std::string_view s) override
    {
        m_current_string += s;
    }

    virtual std::size_t commit_segments() override
    {
        std::size_t string_index = m_ss.size();
        m_ss.push_back(std::move(m_current_string));

        const std::string& s = m_ss.back();
        std::string_view sv(s.data(), s.size());
        m_ss_hash.emplace(sv, string_index);

        return string_index;
    }
};

class my_import_factory : public ss::iface::import_factory
{
    ss_type m_string_pool; // string pool to be shared everywhere.
    my_shared_strings m_shared_strings;
    std::vector<std::unique_ptr<my_sheet>> m_sheets;

public:
    my_import_factory() : m_shared_strings(m_string_pool) {}

    virtual ss::iface::import_shared_strings* get_shared_strings() override
    {
        return &m_shared_strings;
    }

    virtual ss::iface::import_sheet* append_sheet(ss::sheet_t, std::string_view) override
    {
        // Pass the string pool to each sheet instance.
        m_sheets.push_back(std::make_unique<my_sheet>(m_sheets.size(), m_string_pool));
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

int main()
{
    std::filesystem::path input_dir = std::getenv("INPUTDIR");

    my_import_factory factory;
    orcus::orcus_ods loader(&factory);
    loader.read_file(input_dir / "multi-sheets.ods");

    return EXIT_SUCCESS;
}
