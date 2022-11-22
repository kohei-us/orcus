/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "orcus/orcus_csv.hpp"

#include "orcus/csv_parser.hpp"
#include "orcus/stream.hpp"
#include "orcus/spreadsheet/import_interface.hpp"
#include "orcus/config.hpp"
#include "orcus/string_pool.hpp"

#include <cstring>
#include <iostream>

using namespace std;

namespace orcus {

namespace {

constexpr const char* base_sheet_name = "data";

struct header_cell
{
    spreadsheet::row_t row;
    spreadsheet::col_t col;
    std::string_view value;

    header_cell(spreadsheet::row_t _row, spreadsheet::col_t _col, std::string_view _value) :
        row(_row), col(_col), value(_value) {}
};

class max_row_size_reached {};

class orcus_csv_handler
{
public:
    orcus_csv_handler(spreadsheet::iface::import_factory& factory, const orcus::config& app_config) :
        m_factory(factory),
        m_app_config(app_config),
        mp_sheet(nullptr),
        m_sheet(0),
        m_row(0),
        m_col(0) {}

    void begin_parse()
    {
        std::string sheet_name = get_sheet_name();
        mp_sheet = m_factory.append_sheet(m_sheet, sheet_name);
    }

    void end_parse() {}
    void begin_row()
    {
        // Check to see if this row is beyond the max row of the current
        // sheet, and if so, append a new sheet and reset the current row to
        // 0.
        if (m_row >= mp_sheet->get_sheet_size().rows)
        {
            auto csv = std::get<config::csv_config>(m_app_config.data);

            if (!csv.split_to_multiple_sheets)
                throw max_row_size_reached();

            // The next row will be outside the boundary of the current sheet.
            ++m_sheet;
            std::string sheet_name = get_sheet_name();
            mp_sheet = m_factory.append_sheet(m_sheet, sheet_name);
            m_row = 0;

            if (!m_header_cells.empty())
            {
                // Duplicate the header rows from the first sheet.
                for (const header_cell& c : m_header_cells)
                    mp_sheet->set_auto(c.row, c.col, c.value);

                m_row += csv.header_row_size;
            }
        }
    }

    void end_row()
    {
        ++m_row;
        m_col = 0;
    }

    void cell(const char* p, size_t n, bool transient)
    {
        auto csv = std::get<config::csv_config>(m_app_config.data);

        if (m_sheet == 0 && size_t(m_row) < csv.header_row_size)
        {
            std::string_view v{p, n};
            if (transient)
                v = m_pool.intern(v).first;

            m_header_cells.emplace_back(m_row, m_col, v);
        }

        mp_sheet->set_auto(m_row, m_col, {p, n});
        ++m_col;
    }

private:
    std::string get_sheet_name() const
    {
        if (!m_sheet)
            // First sheet has no suffix.
            return base_sheet_name;

        // Add a suffix to keep the sheet name unique.
        std::ostringstream os;
        os << base_sheet_name << '_' << m_sheet;
        return os.str();
    }

private:
    string_pool m_pool;
    std::vector<header_cell> m_header_cells;

    spreadsheet::iface::import_factory& m_factory;
    const config& m_app_config;
    spreadsheet::iface::import_sheet* mp_sheet;
    spreadsheet::sheet_t m_sheet;
    spreadsheet::row_t m_row;
    spreadsheet::col_t m_col;
};

}

struct orcus_csv::impl
{
    spreadsheet::iface::import_factory* factory;

    impl(spreadsheet::iface::import_factory* _factory) : factory(_factory) {}

    void parse(std::string_view stream, const config& conf)
    {
        if (stream.empty())
            return;

        orcus_csv_handler handler(*factory, conf);
        csv::parser_config config;
        config.delimiters.push_back(',');
        config.text_qualifier = '"';
        csv_parser<orcus_csv_handler> parser(stream.data(), stream.size(), handler, config);
        try
        {
            parser.parse();
        }
        catch (const max_row_size_reached&)
        {
            // The parser has decided to end the import due to the destination
            // sheet being full.
        }
        catch (const parse_error& e)
        {
            cout << "parse failed at offset " << e.offset() << ": " << e.what() << endl;
        }
    }
};

orcus_csv::orcus_csv(spreadsheet::iface::import_factory* factory) :
    iface::import_filter(format_t::csv),
    mp_impl(std::make_unique<impl>(factory)) {}

orcus_csv::~orcus_csv() {}

void orcus_csv::read_file(const string& filepath)
{
    file_content fc(filepath.data());
    mp_impl->parse(fc.str(), get_config());
    mp_impl->factory->finalize();
}

void orcus_csv::read_stream(std::string_view stream)
{
    if (stream.empty())
        return;

    mp_impl->parse(stream, get_config());
    mp_impl->factory->finalize();
}

std::string_view orcus_csv::get_name() const
{
    return "csv";
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
