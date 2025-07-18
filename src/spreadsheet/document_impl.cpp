/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "document_impl.hpp"
#include "debug_state_dumper.hpp"
#include "debug_state_context.hpp"

#include <algorithm>
#include <iostream>
#include <fstream>
#include <exception>
#include <sstream>

namespace orcus { namespace spreadsheet { namespace detail {

namespace {

#ifdef _WIN32
std::u16string to_string(const fs::path& p)
{
    return p.u16string();
}
#else
std::string to_string(const fs::path& p)
{
    return p.string();
}
#endif

} // anonymous namespace

sheet_item::sheet_item(document& doc, std::string_view _name, sheet_t sheet_index) :
    name(_name), data(doc, sheet_index) {}

document_impl::document_impl(document& _doc, const range_size_t& sheet_size) :
    doc(_doc),
    context({sheet_size.rows, sheet_size.columns}),
    styles_store(),
    ss_store(context),
    pivots(doc),
    formula_context_to_resolver({{formula_ref_context_t::global, ixion::formula_name_resolver_t::excel_a1}}),
    grammar(formula_grammar_t::xlsx),
    table_store(string_pool_store, context)
{
}

void document_impl::dump(dump_format_t format, const fs::path& outpath) const
{
    if (format == dump_format_t::none)
        return;

    if (format == dump_format_t::check)
    {
        // For this output, we write to a single file.
        std::ostream* ostrm = &std::cout;
        std::unique_ptr<std::ofstream> fs;

        if (!outpath.empty())
        {
            if (fs::is_directory(outpath))
            {
                std::ostringstream os;
                os << "Output file path points to an existing directory.";
                throw std::invalid_argument(os.str());
            }

            // Output to stdout when output path is not given.
            fs = std::make_unique<std::ofstream>(outpath);
            ostrm = fs.get();
        }

        dump_check(*ostrm);
        return;
    }

    if (outpath.empty())
        throw std::invalid_argument("No output directory.");

    if (fs::exists(outpath))
    {
        if (!fs::is_directory(outpath))
        {
            std::ostringstream os;
            os << "A file named '" << outpath << "' already exists, and is not a directory.";
            throw std::invalid_argument(os.str());
        }
    }
    else
        fs::create_directory(outpath);

    switch (format)
    {
        case dump_format_t::csv:
            dump_csv(outpath);
            break;
        case dump_format_t::flat:
            dump_flat(outpath);
            break;
        case dump_format_t::html:
            dump_html(outpath);
            break;
        case dump_format_t::json:
            dump_json(outpath);
            break;
        case dump_format_t::debug_state:
            dump_debug_state(outpath);
            break;
        // coverity[dead_error_line] - following conditions exist to avoid compiler warning
        case dump_format_t::none:
        case dump_format_t::unknown:
            break;
        default:
            ;
    }
}

void document_impl::dump_flat(const fs::path& outdir) const
{
    std::cout << "----------------------------------------------------------------------" << std::endl;
    std::cout << "  Document content summary" << std::endl;
    std::cout << "----------------------------------------------------------------------" << std::endl;
    ss_store.dump(std::cout);

    std::cout << "number of sheets: " << sheets.size() << std::endl;

    for (const std::unique_ptr<detail::sheet_item>& sheet : sheets)
    {
        fs::path outpath{outdir};
        outpath /= std::string{sheet->name};
        outpath.replace_extension(".txt");

        std::ofstream file(outpath.native());
        if (!file)
        {
            std::cerr << "failed to create file: " << outpath << std::endl;
            return;
        }

        file << "---" << std::endl;
        file << "Sheet name: " << sheet->name << std::endl;
        sheet->data.dump_flat(file);
    }
}

void document_impl::dump_html(const fs::path& outdir) const
{
    for (const std::unique_ptr<detail::sheet_item>& sheet : sheets)
    {
        fs::path outpath{outdir};
        outpath /= std::string{sheet->name};
        outpath.replace_extension(".html");

        std::ofstream file(outpath.native());
        if (!file)
        {
            std::cerr << "failed to create file: " << outpath << std::endl;
            return;
        }

        sheet->data.dump_html(file);
    }
}

void document_impl::dump_json(const fs::path& outdir) const
{
    for (const std::unique_ptr<detail::sheet_item>& sheet : sheets)
    {
        fs::path outpath{outdir};
        outpath /= std::string{sheet->name};
        outpath.replace_extension(".json");

        std::ofstream file(outpath.native());
        if (!file)
        {
            std::cerr << "failed to create file: " << outpath << std::endl;
            return;
        }

        sheet->data.dump_json(file);
    }
}

void document_impl::dump_csv(const fs::path& outdir) const
{
    for (const std::unique_ptr<detail::sheet_item>& sheet : sheets)
    {
        fs::path outpath{outdir};
        outpath /= std::string{sheet->name};
        outpath.replace_extension(".csv");

        std::ofstream file(outpath.c_str());
        if (!file)
        {
            std::cerr << "failed to create file: " << outpath << std::endl;
            return;
        }

        sheet->data.dump_csv(file);
    }
}

void document_impl::dump_debug_state(const fs::path& outdir) const
{
    detail::debug_state_context cxt;
    detail::doc_debug_state_dumper dumper{cxt, *this};
    fs::path output_dir{outdir};
    dumper.dump(output_dir);

    for (const std::unique_ptr<detail::sheet_item>& sheet : sheets)
    {
        fs::path outpath = output_dir;
        outpath /= "sheets";
        outpath /= sheet->name;
        fs::create_directories(outpath);
        sheet->data.dump_debug_state(to_string(outpath), sheet->name);
    }

    pivots.dump_debug_state(to_string(outdir));
}

void document_impl::dump_check(std::ostream& os) const
{
    for (const std::unique_ptr<detail::sheet_item>& sheet : sheets)
        sheet->data.dump_check(os, sheet->name);
}

}}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
