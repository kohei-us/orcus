
#include <orcus/xml_namespace.hpp>
#include <orcus/stream.hpp>
#include <orcus/orcus_xml.hpp>

#include <orcus/spreadsheet/factory.hpp>
#include <orcus/spreadsheet/document.hpp>
#include <orcus/spreadsheet/sheet.hpp>

#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;

int main() try
{
    auto inputpath = fs::path{INPUTDIR} / "server-logs.xml";
    orcus::file_content input{inputpath};
    orcus::spreadsheet::range_size_t ssize{200, 10};
    orcus::spreadsheet::document doc{ssize};
    orcus::spreadsheet::import_factory factory(doc);

    orcus::xmlns_repository repo;
    orcus::orcus_xml filter{repo, &factory};

    filter.set_namespace_alias("log", "http://example.com/server-logs");
    filter.set_namespace_alias("meta", "http://example.com/server-logs/meta");

    filter.set_cell_link("/log:serverLogs/@meta:host", "Logs", 0, 1);
    filter.set_cell_link("/log:serverLogs/@meta:date", "Logs", 1, 1);

    filter.start_range("Logs", 3, 0);
    filter.append_field_link("/log:serverLogs/log:entry/@log:id", "ID");
    filter.append_field_link("/log:serverLogs/log:entry/log:level", "Level");
    filter.append_field_link("/log:serverLogs/log:entry/log:service", "Service");
    filter.append_field_link("/log:serverLogs/log:entry/log:message", "Message");
    filter.append_field_link("/log:serverLogs/log:entry/log:timestamp", "Timestamp");
    filter.set_range_row_group("/log:serverLogs/log:entry");
    filter.commit_range();

    filter.append_sheet("Logs");
    filter.read_stream(input.str());

    auto* sheet = doc.get_sheet(0);
    if (!sheet)
        throw std::runtime_error("failed to fetch the first sheet");

    sheet->set_string(0, 0, "Host");
    sheet->set_string(1, 0, "Date");

    sheet->dump_flat(std::cout);

    return EXIT_SUCCESS;
}
catch (const std::exception& e)
{
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
}

