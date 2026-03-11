
//!code-start: headers
#include <orcus/format_detection.hpp>
#include <orcus/stream.hpp>

#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;
//!code-end: headers

int main() try
{
    //!code-start: getenv
    const char* testdir = std::getenv("TESTDIR");
    //!code-end: getenv
    if (!testdir)
    {
        std::cerr << "TESTDIR not defined" << std::endl;
        return EXIT_FAILURE;
    }

    {
        //!code-start: ods
        auto filepath = fs::path{testdir} / "ods" / "raw-values-1" / "input.ods";
        orcus::file_content fc{filepath};

        auto format = orcus::detect(fc.str());
        std::cout << "format: " << format << std::endl;
        //!code-end: ods
    }

    {
        //!code-start: xlsx
        auto filepath = fs::path{testdir} / "xlsx" / "raw-values-1" / "input.xlsx";
        orcus::file_content fc{filepath};

        auto format = orcus::detect(fc.str());
        std::cout << "format: " << format << std::endl;
        //!code-end: xlsx
    }

    {
        //!code-start: xml
        auto filepath = fs::path{testdir} / "xml" / "simple" / "input.xml";
        orcus::file_content fc{filepath};

        auto format = orcus::detect(fc.str());
        std::cout << "format: " << format << std::endl;
        //!code-end: xml
    }

    {
        //!code-start: json
        auto filepath = fs::path{testdir} / "json" / "basic1" / "input.json";
        orcus::file_content fc{filepath};

        auto format = orcus::detect(fc.str());
        std::cout << "format: " << format << std::endl;
        //!code-end: json
    }

    {
        //!code-start: is-ods
        auto filepath = fs::path{testdir} / "ods" / "raw-values-1" / "input.ods";

        orcus::file_content fc{filepath};
        std::cout << "ods? " << orcus::detect(fc.str(), orcus::format_t::ods) << std::endl;
        std::cout << "xlsx? " << orcus::detect(fc.str(), orcus::format_t::xlsx) << std::endl;
        //!code-end: is-ods
    }

    {
        //!code-start: is-xls-xml
        auto filepath = fs::path{testdir} / "xls-xml" / "raw-values-1" / "input.xml";

        orcus::file_content fc{filepath};
        std::cout << "xls-xml? " << orcus::detect(fc.str(), orcus::format_t::xls_xml) << std::endl;
        std::cout << "xml? " << orcus::detect(fc.str(), orcus::format_t::xml) << std::endl;
        std::cout << "json? " << orcus::detect(fc.str(), orcus::format_t::json) << std::endl;
        //!code-end: is-xls-xml
    }

    return EXIT_SUCCESS;
}
catch (...)
{
    return EXIT_FAILURE;
}

