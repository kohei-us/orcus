
#include <orcus/spreadsheet/import_interface.hpp>
#include <orcus/orcus_ods.hpp>

#include <iostream>
#include <memory>
#include <unordered_map>
#include <deque>
#include <filesystem>

namespace ss = orcus::spreadsheet;

enum class cell_value_type { empty, numeric, string };

using ss_type = std::deque<std::string>;
using ss_hash_type = std::unordered_map<std::string_view, std::size_t>;

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

class my_sheet : public ss::iface::import_sheet
{
    cell_value m_cells[100][1000];
    ss::range_size_t m_sheet_size;
    ss::sheet_t m_sheet_index;
    const ss_type& m_string_pool;

public:
    my_sheet(ss::sheet_t sheet_index, const ss_type& string_pool) :
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
    virtual void set_auto(ss::row_t row, ss::col_t col, std::string_view s) override {}

    virtual void set_bool(ss::row_t row, ss::col_t col, bool value) override {}

    virtual void set_date_time(
        ss::row_t row, ss::col_t col,
        int year, int month, int day, int hour, int minute, double second) override {}

    virtual void set_format(ss::row_t row, ss::col_t col, std::size_t xf_index) override {}

    virtual void set_format(
        ss::row_t row_start, ss::col_t col_start, ss::row_t row_end, ss::col_t col_end,
        std::size_t xf_index) override {}

    virtual void fill_down_cells(
        ss::row_t src_row, ss::col_t src_col, ss::row_t range_size) override {}
};

class my_shared_strings : public ss::iface::import_shared_strings
{
    ss_hash_type m_ss_hash;
    ss_type& m_ss;
    std::string m_current_string;

public:
    my_shared_strings(ss_type& ss) : m_ss(ss) {}

    virtual std::size_t add(std::string_view s) override
    {
        auto it = m_ss_hash.find(s);
        if (it != m_ss_hash.end())
            // This string already exists in the pool.
            return it->second;

        // This is a brand-new string.
        return append(s);
    }

    virtual std::size_t append(std::string_view s) override
    {
        std::size_t string_index = m_ss.size();
        m_ss.emplace_back(s);
        m_ss_hash.emplace(s, string_index);

        return string_index;
    }

    // The following methods are for formatted text segments, which we ignore for now.
    virtual void set_segment_bold(bool b) override {}

    virtual void set_segment_font(std::size_t font_index) override {}

    virtual void set_segment_font_color(
        ss::color_elem_t alpha,
        ss::color_elem_t red,
        ss::color_elem_t green,
        ss::color_elem_t blue) override {}

    virtual void set_segment_font_name(std::string_view s) override {}

    virtual void set_segment_font_size(double point) override {}

    virtual void set_segment_italic(bool b) override {}

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

    virtual ss::iface::import_sheet* append_sheet(
        ss::sheet_t sheet_index, std::string_view name) override
    {
        // Pass the string pool to each sheet instance.
        m_sheets.push_back(std::make_unique<my_sheet>(m_sheets.size(), m_string_pool));
        return m_sheets.back().get();
    }

    virtual ss::iface::import_sheet* get_sheet(std::string_view name) override
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
