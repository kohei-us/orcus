# Orcus

Orcus is a C++ library providing standalone file processing filters, focused
primarily on spreadsheet documents.

[![License: MPL 2.0](https://img.shields.io/badge/License-MPL_2.0-brightgreen.svg)](https://opensource.org/licenses/MPL-2.0)
[![Documentation](https://readthedocs.org/projects/orcus/badge/?version=latest)](https://orcus.readthedocs.io/en/latest/)
![C++](https://img.shields.io/badge/C%2B%2B-20-blue?logo=c%2B%2B)

## Import Filters

* Microsoft Excel 2007 XML
* Microsoft Excel 2003 XML
* Open Document Spreadsheet
* Plain Text
* Gnumeric XML
* Generic XML
* Apache Parquet (via Apache Arrow library)

## Low-level Parsers

All parsers are implemented as C++ templates with a handler class passed as a
template argument.

* CSV
* CSS
* XML
* JSON
* YAML (experimental)

## Building

See [CONTRIBUTING.md](CONTRIBUTING.md) for build instructions and dependencies.

## Documentation

* [Official API documentation](https://orcus.readthedocs.io/en/latest/)
